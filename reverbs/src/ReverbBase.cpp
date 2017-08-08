//
//  ReverbBase.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "reverbs/include/ReverbBase.hpp"

SSRverb::ReverbBase::ReverbBase(  const char* name
                 , unsigned n_rev_sources
                 )
: JackPlugin( name, 1, n_rev_sources )
{
    _n_rev_sources = n_rev_sources;
    set_update_callback( ReverbBase::track_rev_sources, this );
    set_reference_callback( SSRverb::ReverbBase::track_reference, this );
}

void SSRverb::ReverbBase::setup_rev_sources()
{
    _mtx.lock();
    if ( is_connected() ) {
        move_reference( _rec_pos[0], _rec_pos[1] );
        
        float x_pos, y_pos;
        char src_name[10];
        for ( unsigned rev = 0; rev < _n_rev_sources; rev++ )
        {
            sprintf( src_name, "rev_%i", rev+1 );
            x_pos = _rec_pos[0] + _radius * cosf( 2.f*M_PI/_n_rev_sources * rev );
            y_pos = _rec_pos[1] + _radius * sinf( 2.f*M_PI/_n_rev_sources * rev );
            
            
            //printf("Setting up reveb source %i at (%f, %f)\n", rev+1, x_pos, y_pos);
            setup_new_source( src_name, x_pos, y_pos, false );
        }
        _rev_srcs_set.store( true );
    }
    _mtx.unlock();
}

void SSRverb::ReverbBase::_update_rev_sources()
{
    _mtx.lock();
    if ( is_connected() && _rev_srcs_set.load() ) {
        float x_pos, y_pos;
        char src_name[10];
        printf("Updating reverb sources to reference = (%f, %f)\n", _rec_pos[0], _rec_pos[1]);
        for ( unsigned rev = 0; rev < _rev_source_ids.size(); rev++ )
        //for ( unsigned rev = 0; rev < _n_rev_sources; rev++ )
        {
            sprintf( src_name, "rev_%i", rev );
            x_pos = _rec_pos[0] + _radius * cosf( 2.f*M_PI/_n_rev_sources * rev );
            y_pos = _rec_pos[1] + _radius * sinf( 2.f*M_PI/_n_rev_sources * rev );
            
            move_source( _rev_source_ids[rev], x_pos, y_pos );
            //move_source( rev+1, x_pos, y_pos );
            printf("Moving reveb source %i to (%f, %f)\n", rev+1, x_pos, y_pos);
        }
    }
    _mtx.unlock();
}

void SSRverb::ReverbBase::end()
{
    deactivate();
    stop();
}

void SSRverb::ReverbBase::remove_rev_sources()
{
    if ( is_connected() && _rev_srcs_set.load() ) {
        for ( unsigned rev = 0; rev < _rev_source_ids.size(); rev++) {
        //for ( unsigned rev = 0; rev < _n_rev_sources; rev++ ) {
            delete_source( _rev_source_ids[rev] );
            //delete_source( rev+1 );
            printf("Deleting reverb source %i\n", rev+1);
        }
        _rev_srcs_set.store( false );
        _rev_source_ids.clear();
    }
}

void SSRverb::ReverbBase::set_rec_pos( Vector3D new_pos )
{
    _rec_pos = new_pos;
    move_reference( new_pos[0], new_pos[1] );
}

void SSRverb::ReverbBase::set_src_pos( Vector3D new_pos )
{
    _src_pos = new_pos;
}

void SSRverb::ReverbBase::set_tracked_source( unsigned source_id )
{
    _tracked_source_id = source_id;
}

void SSRverb::ReverbBase::set_radius( float new_radius )
{
    _radius = new_radius;
    _update_rev_sources();
}

std::vector< unsigned short > SSRverb::ReverbBase::get_rev_ids()
{
    return _rev_source_ids;
}

bool SSRverb::ReverbBase::is_rev_source( unsigned short id )
{
    if ( std::find( _rev_source_ids.begin(), _rev_source_ids.end(), id ) != _rev_source_ids.end() ) {
        return true;
    }
    else return false;
}

SSRverb::ReverbBase::~ReverbBase()
{
    //printf("ReverbBase destructor called..");
    remove_rev_sources();
    stop();
    disconnect();
    deactivate();
}

void SSRverb::ReverbBase::connect_to_ssr()
{
    if ( is_active() && _rev_srcs_set.load() )
    {
        char out_port_name[100];
        char in_port_name[100];
        int success;
        for ( unsigned prt = 1; prt <= _n_out_ports; prt++ ) {
            sprintf(out_port_name, "%s:out_%i", jack_get_client_name(_jack_client), prt );
            sprintf(in_port_name, "BinauralRenderer:in_%i", prt+1 );
            success = jack_connect( _jack_client, out_port_name, in_port_name);
            printf("Connecting: %s <-> %s\t%s\n", out_port_name, in_port_name, success == 0 ? "Succeeded" : "Failed" );
            
    //        sprintf(in_port_name, "WFS-Renderer:in_%i", prt+1 );
    //        jack_connect( _jack_client, out_port_name, in_port_name);
    //        sprintf(in_port_name, "AAP-Renderer:in_%i", prt+1 );
    //        jack_connect( _jack_client, out_port_name, in_port_name);
    //        sprintf(in_port_name, "VBAP-Renderer:in_%i", prt+1 );
    //        jack_connect( _jack_client, out_port_name, in_port_name);
        }
    }
}
