//
//  Randomizer.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef Randomizer_hpp
#define Randomizer_hpp

#include <stdio.h>
#include <sndfile.h>
#include <vector>

#include "laproque/include/laproque.hpp"

// First crossover frequency.
const float FCOF = 62.5f;

namespace SSRverb {

/**
@class Randomizer 
The Randomizer takes a single channel impulse response and 
distributes it amog a number of reverberation sources on a circle.
@param file_path Path to a single channel impulse response.
@param n_sources The desired number of reverberation sources.
*/
class Randomizer
{
public:
    Randomizer( const char* file_path, unsigned n_sources = 8 );
    ~Randomizer();
    
    /** @brief Starts the impulse response spatialization process. */
    void create_spacial_imp_resp( bool write_wavs = false );
    
    float** get_spac_imp_resps();
    
    long long get_ir_length();
    
private:
    unsigned _n_sources;
    long long _ir_length;
    long long _max_padding;
    unsigned _sample_rate;
    
    float* _source_angles;
    float* _mono_imp_resp = nullptr;
    float** _spac_imp_resps = nullptr;
    
    bool _allocated = false;
    
    laproque::Filterbank* _filterbank;
    
    void _read_file( const char* file_path );
    void _allocate_ir_data();
    
    void _distribute_samples();
    
    void _remove_direct();
    
    std::vector<long long> _find_peaks( float* data, float *data_end, float threshold, long long n_peaks = 0 );
    
    std::vector< float > _co_freqs;
    unsigned _n_bands;
    
    void _energy_equalization( float** equalized, long long win_size );
};

} // namespace SSRverb

#endif /* Randomizer_hpp */
