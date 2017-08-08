//
//  FDNecode.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef FDNecode_hpp
#define FDNecode_hpp


#include <stdio.h>
#include <vector>
#include <atomic>
#include <random>

#include "laproque/include/JackPlugin.hpp"
#include "laproque/include/FilteredDelay.hpp"
#include "reverbs/include/Room.hpp"
#include "ssrface/include/SceneManager.hpp"
#include "Matrix.h"
#include "reverbs/ismverb/include/ISMverb.hpp"

namespace SSRverb {
/**
 @class FDN Implementation of a Feedback Delay Network with choosable number of feedback paths, sample rate and number of output channels.
 */
class FDN
{
public:
    /**
     @param sample_rate Sample rate used in processing.
     @param n_fbpaths Number of feedback paths to be used. Must be power of 2 or equal 24.
     */
    FDN( unsigned sample_rate, unsigned n_fbpaths = 16, unsigned n_rev_sources = 8 );
    ~FDN();
    
    /**
     @brief Process the samples in input and write results to output.
     @param input Pointer to array with input samples.
     @param outputs Pointer to arrays where the resulting channels are written to.
     @param n_frames Number of samples to be processed.
     */
    void process( float* input, float** outputs, unsigned n_frames );
    
    /** 
     @brief Set the reverberation time of one frequency band.
     @param t60_value Reverberation time.
     @param band_idx Index of the according frequency band.
     */
    void set_t60( float t60_value, unsigned band_idx );
    
    /**
     @brief Set the crossover frequencies of the filterbanks in the feedback paths.
     @param co_freqs Vector containing the crossover frequencies.
     */
    void set_co_freqs( std::vector< float > co_freqs );
    
    /**
     @brief Set the dimensions of a room, which the FDN tries mimic.
     @param x Dimension in x-direction.
     @param y Dimension in y-direction.
     @param z Dimension in z-direction.
     */
    void set_boundries( float x, float y, float  z );
    
    /** @brief Computes all internal values to employ the current settings. */
    void update_t60();
    
private:
    // FDN properties
    const unsigned _n_fbpaths;
    const unsigned _n_rev_sources;
    float _path_weight = 1.f;
    float _boundries[3]{5.f, 7.f, 3.5f};
    
    //unsigned _min_delay = 65535;
    
    // Sample buffer
    float** _delay_outs;
    float** _matrix_outs;
    void _reset_buffers();
    
    laproque::FilteredDelay*  _delays;
    
    float _t60_values[3]{2.f, 1.f, .2f};
    
    std::mt19937 _mt{ std::random_device{}() };
    std::uniform_real_distribution<> _noise{-0.1, 0.1};
    
    void _compute_delays();
    
    Matrix _fb_matrix;
    
    unsigned _sample_rate;
    const unsigned _intern_buff_size = 1024;
    
};
    
} // namspace SSRverb

#endif /* FDNecode_hpp */
