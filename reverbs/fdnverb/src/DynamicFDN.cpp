//
//  DynamicFDN.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "DynamicFDN.hpp"
#include "reverbs/include/Vector3D.hpp"

const SSRverb::Vector3D DFDN_ROOM_INIT{5.f, 7.f, 3.2};

SSRverb::DynamicFDN::DynamicFDN( unsigned n_rev_sources )
: ReverbBase( "ISMFDNreverb", n_rev_sources ),
  _fdn( _sample_rate, 24, n_rev_sources ),
  _ism( DFDN_ROOM_INIT[0], DFDN_ROOM_INIT[1], DFDN_ROOM_INIT[2], 4, _sample_rate, _block_size )
{
    set_update_callback( ISMverb::update_src_pos, &_ism );
    
    _internal_buffers = new float*[_n_rev_sources];
    for ( unsigned src = 0; src < _n_rev_sources; src++ ) {
        _internal_buffers[src] = new float[_block_size];
    }
    
    set_rec_pos( DFDN_ROOM_INIT/2 );
}

SSRverb::DynamicFDN::~DynamicFDN()
{
    deactivate();
    for ( unsigned src = 0; src < _n_rev_sources; src++ ) {
        delete [] _internal_buffers[src];
    }
    delete [] _internal_buffers;
}

void SSRverb::DynamicFDN::render_audio(
                       jack_nframes_t n_frames
                       , laproque::sample_t **in_buffers
                       , laproque::sample_t **out_buffers
                       )
{
    unsigned prt, idx;
    
    _n_remaining = n_frames;
    
    while ( _n_remaining ) {
        _n_remaining < _block_size ? _n_ready = _n_remaining : _n_ready = _block_size;
        
        _fdn.process( in_buffers[0], out_buffers, _n_ready );
        _ism.process( in_buffers[0], _internal_buffers, _n_ready );
        
        for ( prt = 0; prt < _n_rev_sources; prt++ )
        {
            for ( idx = 0; idx < _n_ready; idx++ ) {
                out_buffers[prt][idx] =
                    ( out_buffers[prt][idx] * _fdn_ism_mix.load() +
                      _internal_buffers[prt][idx] * (1-_fdn_ism_mix.load())
                    ) * _gain.load();
            }
        }
        
        
        _n_remaining -= _n_ready;
        in_buffers[0] += _n_ready;
        
        for ( prt = 0; prt < _n_out_ports; prt++ ) {
            out_buffers[prt] += _n_ready;
        }
    }
    
    for ( prt = 0; prt < _n_out_ports; prt++ ) {
        out_buffers[prt] -= n_frames;
    }
    in_buffers[0] -= n_frames;
}

bool SSRverb::DynamicFDN::connect()
{
    bool result = SceneManager::connect();
    if ( result ) {
        set_rec_pos( _rec_pos );
    }
    return result;
}

void SSRverb::DynamicFDN::set_rec_pos( Vector3D new_pos )
{
    ReverbBase::set_rec_pos( new_pos );
    _ism.set_receiver( new_pos );
}

void SSRverb::DynamicFDN::set_src_pos( Vector3D new_pos )
{
    move_reference( new_pos[0], new_pos[1]);
    _ism.set_source( new_pos );
}

void SSRverb::DynamicFDN::set_tracked_source( unsigned source_id, float x, float y )
{
    ReverbBase::set_tracked_source( source_id );
    _ism.set_tracked_source( source_id );
    _ism.set_source( Vector3D( x, y, _ism.get_source()[2] ) );
}

void SSRverb::DynamicFDN::set_gain( float new_gain )
{
    _gain.store( new_gain );
}

void SSRverb::DynamicFDN::set_fdn_ism_mix( float new_mix )
{
    _fdn_ism_mix.store( new_mix );
}

void SSRverb::DynamicFDN::set_ism_gain( float new_gain )
{
    _ism_gain.store( new_gain );
}

void SSRverb::DynamicFDN::set_fdn_gain( float new_gain )
{
    _fdn_gain.store( new_gain );
}

void SSRverb::DynamicFDN::set_room_size( float x, float y, float z )
{
    _fdn.set_boundries( x, y, z );
    _ism.set_room_dimensions( y, x, z );
}

void SSRverb::DynamicFDN::set_t60( float t60_value, unsigned band_idx )
{
    if ( band_idx < _n_bands )
    {
        _t60_times[band_idx] = t60_value;
        
        _fdn.set_t60( t60_value, band_idx );
        _ism.set_t60( t60_value, band_idx );
        
    }

}

void SSRverb::DynamicFDN::set_co_freqs( std::vector<float> co_freqs )
{
    _fdn.set_co_freqs( co_freqs );
    _ism.set_co_freqs( co_freqs );
}

void SSRverb::DynamicFDN::set_tracking( bool status )
{
    // Move tracked source in front of receiver.
    //_ism.set_source( _ism.get_receiver() + Vector3D{0.f, 1.f, 0.f} );
    _ism.set_tracking( status );
}

bool SSRverb::DynamicFDN::get_tracking()
{
    return _ism.get_tracking();
}
