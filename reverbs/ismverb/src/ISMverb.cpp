//
//  ISMverb.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "ISMverb.hpp"
#include <cstring>
#include <random>

SSRverb::ISMverb::ISMverb(
                 float x
                 , float y
                 , float z
                 , unsigned order
                 , unsigned sample_rate
                 , unsigned block_size
                 )
: _room(x, y, z)
{
    // Store data.
    _order = order;
    _n_mirr_sources = Room::get_n_mirr_src( order );
    _sample_rate = sample_rate;
    _block_size = block_size;
    
    // Initialize valid positions of source an receiver
    _src_pos = Vector3D{x/3.f, y/3.f, z/3.f};
    _rec_pos = Vector3D{2.f*x/3.f, 2.f*y/3.f, 2.f*z/3.f};
    
    // Create vector with number of mirror sources in every order.
    _sources_in_order = new unsigned[_order];
    for ( unsigned ord = 1; ord <= order; ord++ ) {
        _sources_in_order[ord-1] = Room::get_n_mirr_src( ord ) - Room::get_n_mirr_src( ord-1  );
        //printf("Mirror sources in order %i: %i\n", ord, _sources_in_order[ord-1]);
    }
    
    _make_allocations();
    
    // Calculate reverb position angles.
    float x_pos, y_pos, radius = 1.2f;
    for ( unsigned rev = 0; rev < _n_rev_sources; rev++ ) {
        x_pos = _rec_pos[0] + radius * cosf( 2.f*M_PI/_n_rev_sources * rev );
        y_pos = _rec_pos[1] + radius * sinf( 2.f*M_PI/_n_rev_sources * rev );
        
        // Store the  azimuth of the reverb source in respect to the listener.
        _rev_source_angles[rev] = _rec_pos.azimuth_to( Vector3D{x_pos, y_pos, 0.f} );
        //if ( _rev_source_angles[rev] < 0.f ) _rev_source_angles[rev] += 2*M_PI;
        //printf("%f, ", _rev_source_angles[rev]);
    }
    
    _max_anglular_distance = fabsf(_rev_source_angles[0] - _rev_source_angles[1]) / 2.f;
    
    // install delays
    _band_weights = new float*[_order];
    
    unsigned ord, band;
    for ( ord = 0; ord < _order; ord++)
    {
        _band_weights[ord] = new float[ISM_CO_FREQS.size()];
        // Compute band weights for current order
        for ( band = 0; band < ISM_BAND_WEIGHTS.size(); band++ ) {
            _band_weights[ord][band] = powf( ISM_BAND_WEIGHTS[band], float(ord+1) );
        }
        
        // Set preferences of filterbanks.
        _filterbanks[ord]->set_sample_rate( _sample_rate );
        _filterbanks[ord]->set_co_freqs( ISM_CO_FREQS );
    }
    
    // Initialize delays
    _update_delays();
    
}

void SSRverb::ISMverb::_make_allocations()
{
    unsigned ord, rev, band;
    
    // Create a filter for every order in every reverb source.
    //printf( "\n Allocating MultiDelays form ISM: Sources: %i, Order: %i\n", _n_rev_sources, _order );
    
    _delays = new laproque::FadingMultiDelay**[_n_rev_sources];
    _delay_counters = new unsigned*[_n_rev_sources];
    _delay_values = new unsigned long**[_n_rev_sources];
    _delay_weights = new float**[_n_rev_sources];
    
    for ( rev = 0; rev < _n_rev_sources; rev++ )
    {
        _delays[rev] = new laproque::FadingMultiDelay*[_order];
        _delay_counters[rev] = new unsigned[_order];
        _delay_values[rev] = new unsigned long*[_order];
        _delay_weights[rev] = new float*[_order];
        
        for ( ord = 0; ord < _order; ord++ )
        {
            _delay_counters[rev][ord] = 0;
            _delays[rev][ord] = new laproque::FadingMultiDelay( 10000 );
            _delay_values[rev][ord] = new unsigned long[ _sources_in_order[_order-1] ];
            _delay_weights[rev][ord] = new float[ _sources_in_order[_order-1] ];
        }
    }
    
    _filterbanks = new laproque::Filterbank*[_order];
    _band_weights = new float*[_order];
    
    for ( ord = 0; ord < _order; ord++ )
    {
        //printf("ALLOCATING: mirror sources in current order: %i\n", _sources_in_order[ord]);
        _filterbanks[ord] = new laproque::Filterbank(  ISM_CO_FREQS, _sample_rate );
    }
    
    _mirror_sources = Room::prepare_mirror_vector( _order );
    _one_order = new Vector3D[_sources_in_order[_order-1]];
    
    _delay_output = new float[_block_size];
    
    _band_buffers = new float*[_n_freq_bands];
    for ( band = 0; band < _n_freq_bands; band++) {
        _band_buffers[band] = new float[_block_size];
    }
    
    _internal_buffer = new float[_block_size];
}

SSRverb::ISMverb::~ISMverb()
{
    
    Room::dispose_mirror_vector( _mirror_sources, _order );
    
    delete [] _one_order;
    
    for (unsigned ord = 0; ord < _order; ord++)
    {
        delete [] _band_weights[ord];
        
    }
    
    delete [] _band_weights;
    
    for ( unsigned rev = 0; rev < _n_rev_sources; rev++ ) {
        for ( unsigned ord = 0; ord < _order; ord++ ) {
            delete _delays[rev][ord];
            delete [] _delay_values[rev][ord];
            delete [] _delay_weights[rev][ord];
        }
        delete [] _delays[rev];
        delete [] _delay_counters[rev];
        delete [] _delay_values[rev];
        delete [] _delay_weights[rev];
    }
    delete [] _delays;
    delete [] _delay_counters;
    
    delete [] _delay_values;
    delete [] _delay_weights;
    
    delete [] _delay_output;
    
    delete [] _sources_in_order;
    
    
    for ( unsigned band = 0; band < _n_freq_bands; band++) {
        delete [] _band_buffers[band];
    }
    delete [] _band_buffers;
    
    delete [] _internal_buffer;
}


void SSRverb::ISMverb::_update_delays()
{
 
    // Mute if source is outside of room.
    bool in_scene = _src_pos[0] < _room.get_x_size()
                 && _src_pos[1] < _room.get_y_size();
    
    in_scene &= _src_pos[0] > 0
             && _src_pos[1] > 0;
    
    unsigned rev, ord, src;
    
    // Silence in case source is not in scene;
    if ( !in_scene )
    {
        for ( rev = 0; rev < _n_rev_sources; rev++) {
            for ( ord = 0; ord < _order; ord++) {
                //printf("SILENCING %i SOURCES, rev: %i ord: %i\n", _delay_counters[rev][ord], rev, ord );
                _delays[rev][ord]->clear_delays();
            }
        }
        return;
    }
    
    
    float distance, weight, closest_weight, neighbor_weight, angle;
    float angle_diff = 1e6;
    float abs_angle_diff = 1e-6;
    
    int neighbor, closest_reverb = 0;
    
    long samples_delay;
    
    // TEMP: SIGN RANDOMIZER
//    std::mt19937 gen( time(0) );
//    std::uniform_int_distribution<int> dis(-1, 1);
    
    // Calculate direct distance for relative compensation.
    float direct_distance = (_rec_pos - _src_pos).get_length();
    
    // Compute the positions of all mirror sources
    _room.mirror_point( _src_pos, _order, _mirror_sources );
    
    // Loop through orders of reflections
    for ( ord = 0; ord < _order; ord++)
    {
        // Reset counters.
        for ( rev = 0; rev < _n_rev_sources; rev++ ) {
            _delay_counters[rev][ord] = 0;
        }
        
        // Loop through sources in this order
        Room::extract_order( ord+1, _mirror_sources, _order, _one_order );
        for ( src = 0; src < _sources_in_order[ord]; src++)
        {
            // Compute relevant properties of this mirror source.
            distance = _one_order[src].distance_to( _rec_pos );
            angle = _rec_pos.azimuth_to( _one_order[src] );
            weight = std::min( 1.f / distance, 1.f);
            samples_delay = roundf( (distance-direct_distance) / 343. * _sample_rate);
            
            
            // Find reverb source with closest azimuth.
            for ( rev = 0; rev < _n_rev_sources; rev++)
            {
                angle_diff = angle - _rev_source_angles[rev];
                abs_angle_diff = fabsf( angle_diff );
                // Maximum angular distance of two points on a circle is PI
                if ( abs_angle_diff > M_PI ) abs_angle_diff= 2*M_PI - abs_angle_diff;
                
                if ( abs_angle_diff <= _max_anglular_distance ) {
                    closest_reverb = rev;
                    
                    break;
                }
            }
            //printf("%f\t%i\n", angle_diff, closest_reverb);
            
            // Fade between the neighboring sources.
            closest_weight = weight * (1.f - (abs_angle_diff / _max_anglular_distance));
            
            // Install delay to next neighbor.
            neighbor = closest_reverb + sign(angle_diff);
            if ( neighbor < 0 || neighbor == _n_rev_sources) neighbor = 0;
            neighbor_weight = weight * (abs_angle_diff / _max_anglular_distance);
            
            // Store delay and weight values.
            _delay_values [closest_reverb][ord][_delay_counters[closest_reverb][ord]] = samples_delay;
            _delay_weights[closest_reverb][ord][_delay_counters[closest_reverb][ord]] = closest_weight;
            _delay_counters[closest_reverb][ord]++;
            
            _delay_values [neighbor][ord][_delay_counters[neighbor][ord]] = samples_delay;
            _delay_weights[neighbor][ord][_delay_counters[neighbor][ord]] = neighbor_weight;
            _delay_counters[neighbor][ord]++;
            
        }
        
        // Apply new values in delay modules of this order.
        for ( rev = 0; rev < _n_rev_sources; rev++ ) {
            _delays[rev][ord]->set_delays(  _delay_values[rev][ord]
                                          , _delay_weights[rev][ord]
                                          , _delay_counters[rev][ord]
                                          );
        }
    }
}


void SSRverb::ISMverb::process(
                      float *input
                      , float **outputs
                      , unsigned long n_frames
                      )
{
    if ( _has_changed.load() ) _update_delays();
    
    unsigned rev, idx, ord, band;
    
    for ( rev = 0; rev < _n_rev_sources; rev++ ) {
        for ( idx = 0; idx < n_frames; idx++ ) {
            outputs[rev][idx] = 0.f;
        }
    }
    
    for ( ord = 0; ord < _order; ord++ )
    {
        // Process Filterbank in this oder.
        _filterbanks[ord]->process( input, _band_buffers, n_frames );
        for ( idx = 0; idx < n_frames; idx++ )
        {
            _internal_buffer[idx] = 0.f;
            // Apply band weights according to order
            for ( band = 0; band < _n_freq_bands; band++ ) {
                _internal_buffer[idx] += _band_buffers[band][idx] * _band_weights[ord][band];
            }
        }
        
        for ( rev = 0; rev < _n_rev_sources; rev++ )
        {
            // Process delay
            _delays[rev][ord]->process( _internal_buffer, _delay_output, n_frames );
            
            // Add to output buffer.
            for ( idx = 0; idx < n_frames; idx++ ) {
                outputs[rev][idx] += _delay_output[idx];
            }
        }
    }
    
    _has_changed.store( false );
}

void SSRverb::ISMverb::set_source( Vector3D source )
{
    _src_pos = source;
    _has_changed.store( true );
}

void SSRverb::ISMverb::set_receiver( Vector3D receiver )
{
    _rec_pos = receiver;
    _has_changed.store( true );
}

SSRverb::Vector3D SSRverb::ISMverb::get_source()
{
    return _src_pos;
}

SSRverb::Vector3D SSRverb::ISMverb::get_receiver()
{
    return _rec_pos;
}

void SSRverb::ISMverb::set_room_dimensions( float x, float y, float z )
{
    _room.set_dimensions( x, y, z );
    _has_changed = true;
}

void SSRverb::ISMverb::set_co_freqs( std::vector<float> co_freqs )
{
    for ( unsigned ord = 0; ord < _order; ord++) {
        _filterbanks[ord]->set_co_freqs( co_freqs );
    }
}

void SSRverb::ISMverb::set_band_weight( float weight, unsigned band_idx )
{
    for ( unsigned ord = 0; ord < _order; ord++)
    {
        
    }
}

void SSRverb::ISMverb::set_tracked_source( unsigned int source_id )
{
    _tracked_source_id = source_id;
}

void SSRverb::ISMverb::set_tracking( bool status )
{
    _tracking_active.store( status );
}

bool SSRverb::ISMverb::get_tracking()
{
    return _tracking_active.load();
}

void SSRverb::ISMverb::set_t60( float t60_value, unsigned band_idx )
{
    // Estimate using sabine.
    
    float weight_estimate = 1 - ( 24.f * logf(10.f) * _room.get_volume() ) /
                                ( 343.f * t60_value * _room.get_surface() );
    
    //printf("RT = %f \t w = %f\n", t60_value, weight_estimate);
    
    for ( unsigned ord = 0; ord < _order; ord++ ) {
        _band_weights[ord][band_idx] = powf( weight_estimate, float(ord+1) );
    }
    
}
