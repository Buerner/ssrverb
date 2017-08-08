//
//  FDNecode.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "FDN.hpp"
#include "prime.h"
#include "tools.h"

#include <math.h>

SSRverb::FDN::FDN( unsigned sample_rate, unsigned n_fbpaths, unsigned n_rev_sources ) :
_n_fbpaths( n_fbpaths ), _n_rev_sources( n_rev_sources )
{
    
    _delays = new laproque::FilteredDelay[_n_fbpaths];
    
    _path_weight = 1.f / _n_rev_sources;
    
    _sample_rate = sample_rate;
    
    _fb_matrix.resize( _n_fbpaths );
    _fb_matrix = hadamard( _n_fbpaths, 1.f/sqrtf(float(_n_fbpaths)) );
    
    _matrix_outs = new float*[_n_fbpaths];
    _delay_outs = new float*[_n_fbpaths];
    
    for ( unsigned path = 0; path < _n_fbpaths; path++ ) {
        _matrix_outs[path] = new float[_intern_buff_size];
        _delay_outs[path] = new float[_intern_buff_size];
    }
    
    _reset_buffers();
    
    _compute_delays();
    
    // Initialize T60 values;
    set_t60( _t60_values[0], 0 );
    set_t60( _t60_values[1], 1 );
    set_t60( _t60_values[2], 2 );
}

SSRverb::FDN::~FDN()
{
    for ( unsigned path = 0; path < _n_fbpaths; path++ ) {
        delete [] _matrix_outs[path];
        delete [] _delay_outs[path];
    }
    
    delete [] _matrix_outs;
    delete [] _delay_outs;
    
    delete [] _delays;
}

void SSRverb::FDN::process( float* input, float** outputs, unsigned n_frames )
{
    unsigned idx, path, row, col, path_map, out;
    
    for ( idx = 0; idx < n_frames; idx++ ) {
        // Set outputs to zero
        for ( out = 0; out < _n_rev_sources; out++) {
            outputs[out][idx] = 0.f;
        }
        
        for ( path = 0; path < _n_fbpaths; path++) {
            path_map = path  % _n_rev_sources;
            
            _delay_outs[path][0] = _delays[path]( _matrix_outs[path][0] + input[idx] );
            outputs[path_map][idx] += _delay_outs[path][0] * _path_weight;
        }
        
        // Apply Feedback matrix
        for ( row = 0; row < _n_fbpaths; row++ )
        {
            _matrix_outs[row][0] = 0.f;
            for ( col = 0; col < _n_fbpaths; col++ )
            {
                _matrix_outs[row][0] += _fb_matrix[row][col] * _delay_outs[col][0];
            }
        }
    }
    
}

void SSRverb::FDN::_compute_delays()
{
    /* ========== DELAY VALUES ACORDING TO ROOM DIMENSIONS ========== */
    
    float rand_val;
    
    const float dimension_mean = (_boundries[0] + _boundries[1] + _boundries[2]) / 3.f;
    
    unsigned this_delay;
    for ( unsigned path = 0; path < _n_fbpaths; path++ )
    {
        rand_val = dimension_mean*_noise(_mt);
        this_delay = unsigned(roundf( (_boundries[path%3] + rand_val) / 343.f * _sample_rate));
        _delays[path].set_delay( this_delay  );
    }    
    
    /* ========== PRIME POWER DELAY VALUES ========== */
    // convert from meters to travel time in samples
//    float max_delay = round( (max(_boundries[0], _boundries[1]) / 343.f) * _sample_rate );
//    float min_delay = round((min(_boundries[0], _boundries[1]) / 343.f) * _sample_rate );
//    
//    // get as many prime numbers as there are feedback paths
//    long* primes = get_n_primes( _n_fbpaths );
//    
//    _min_delay = 65535;
//    unsigned this_delay;
//    for ( unsigned i = 0; i < _n_fbpaths; i++ )
//    {
//        // exponential distribution of delays between boundries
//        this_delay = min_delay * pow( (float(max_delay)/float(min_delay)),
//                                     (float(i+1)/float(_n_fbpaths-1)) );
//        // prime power multiplicity
//        float multiplicity = floor(0.5 + log10(this_delay)/log10(primes[i]));
//        
//        this_delay = int(powf(primes[i], multiplicity));
//        
//        _delays[i].set_delay( this_delay );
//        
//        if (this_delay < _min_delay ) _min_delay = this_delay;
//    }
}

void SSRverb::FDN::set_boundries( float x, float y, float z )
{
    _boundries[0] = x;
    _boundries[1] = y;
    _boundries[2] = z;
    
    _compute_delays();
    
    for ( unsigned band = 0; band < 3; band++ ) {
        set_t60( _t60_values[band], band );
    }
}

void SSRverb::FDN::set_t60( float t60_value, unsigned band_idx )
{
    float weight;
    for ( unsigned path = 0; path < _n_fbpaths; path++ )
    {
        weight = exp( (-3.f * logf(10.f) * _delays[path].get_delay()) /
                     (t60_value * _sample_rate) );
        _delays[path].set_band_weight( weight, band_idx );
    }
    _t60_values[band_idx] = t60_value;
}

void SSRverb::FDN::set_co_freqs( std::vector<float> co_freqs )
{
    for (unsigned path = 0; path < _n_fbpaths; path++ )
    {
        _delays[path].set_co_freqs( co_freqs );
    }
}

void SSRverb::FDN::_reset_buffers()
{
    for ( unsigned path = 0; path < _n_fbpaths; path++ ) {
        for ( unsigned idx = 0; idx < _intern_buff_size; idx++ ) {
            _delay_outs[path][idx] = 0.f;
            _matrix_outs[path][idx] = 0.f;
        }
    }
}



