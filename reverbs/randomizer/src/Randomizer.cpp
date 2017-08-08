//
//  Randomizer.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include <math.h>
#include <random>
#include <algorithm>

#include "Randomizer.hpp"

SSRverb::Randomizer::Randomizer( const char* file_path, unsigned n_sources )
{
    _n_sources = n_sources;
    
    _source_angles = new float[_n_sources];
    for ( unsigned src = 0; src < _n_sources; src++ )
    {
        _source_angles[src] = (2.f* M_PI / _n_sources) * src;
    }
    
    _read_file( file_path );
    
    // Compute cutoff frequencies. Number of octaves depend on sampling frequency.
    _n_bands = unsigned( ceilf( log2f( _sample_rate / (2.f * FCOF ) ) ) ) + 1;
    
    _co_freqs.resize( _n_bands-1 );
    _co_freqs[0] = FCOF / sqrtf(2.f);
    for ( unsigned idx = 1; idx < _n_bands; idx++) {
        _co_freqs[idx] = FCOF * powf( 2, float(idx) ) / sqrtf(2.f);
    }
    
    _filterbank = new laproque::Filterbank(_co_freqs);
}

SSRverb::Randomizer::~Randomizer()
{
    delete [] _source_angles;
    
    if ( _allocated ) {
        for ( unsigned src = 0; src < _n_sources; src++ )
        {
            _spac_imp_resps[src] -= _max_padding;
            delete [] _spac_imp_resps[src];
        }
        delete [] _spac_imp_resps;
    }
    
    delete _filterbank;
}

void SSRverb::Randomizer::_read_file( const char * file_path )
{
    SF_INFO audio_format;
    SNDFILE* audio_file = sf_open( file_path, SFM_READ, &audio_format );
    
    _ir_length = audio_format.frames;
    _sample_rate = audio_format.samplerate;
    
    _allocate_ir_data();
    
    sf_read_float( audio_file, _mono_imp_resp, _ir_length );
    
    sf_close( audio_file );
    

}

void SSRverb::Randomizer::_allocate_ir_data()
{
    // The internal spacial responses are zero padded.
    _max_padding = _ir_length;
    
    _mono_imp_resp = new float[_ir_length + 2*_max_padding];
    _mono_imp_resp += _max_padding;
    
    _spac_imp_resps = new float*[_n_sources];
    for ( unsigned src = 0; src < _n_sources; src++ )
    {
        _spac_imp_resps[src] = new float[_ir_length + 2*_max_padding];
        // Initialize with zeros
        for ( long long idx = 0; idx < _ir_length + 2*_max_padding; idx++ ) {
            _spac_imp_resps[src][idx] = 0.f;
        }
        
        // Move pointer to actual beginning i.e. after leading zeros.
        _spac_imp_resps[src] += _max_padding;
    }
    
    _allocated = true;
}

void SSRverb::Randomizer::create_spacial_imp_resp( bool write_wavs )
{
    unsigned src, band;
    long long idx;
    
    _remove_direct();
    
    _distribute_samples();
    
    SF_INFO audio_format;
    SNDFILE* audio_file;

    // audio_format.frames = _ir_length;
    // audio_format.samplerate = _sample_rate;
    // audio_format.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    // audio_format.frames = 1;
    // audio_format.channels = 1;

    // audio_file = sf_open( "no_direct.wav", SFM_WRITE, &audio_format );
    
    // sf_write_float( audio_file, _mono_imp_resp, _ir_length );
    // sf_close( audio_file );
    
    // Allocate memory for equalized data.
    float** equalized = new float*[_n_sources];
    for ( src = 0; src < _n_sources; src++ ) {
        equalized[src] = new float[_ir_length + 2*_max_padding];
        equalized[src] += _max_padding;
    }
    
    // Allocate memory for bandsplit data.
    float*** freq_bands = new float**[_n_sources];
    for ( src = 0; src < _n_sources; src++ ) {
        freq_bands[src] = new float*[_n_bands];
        for ( band = 0; band < _n_bands; band++ ) {
            freq_bands[src][band] = new float[_ir_length+_max_padding];

        }
    }
    
    // Allocate memory for results.
    float** results = new float*[_n_sources];
    for ( src = 0; src < _n_sources; src++ ) {
        results[src] = new float[_ir_length];
    }
    
    for ( src = 0; src < _n_sources; src++ ) {
        for ( idx = 0; idx < _ir_length; idx++ ) {
            results[src][idx] = 0.f;
        }
    }
    
    long long win_size;
    for ( band = 0; band < _n_bands; band++ )
    {
        if (band == 0) win_size = _ir_length;
        else win_size = round(3.f /_co_freqs[band-1] * _sample_rate);
        // Odd sin window size for perfect reconstruction.
        win_size += win_size%2 - 1;
        
        _energy_equalization( equalized, win_size );
        
        // Split into frequency bands.
        // Unecessary to do the entire filterbank every time.
        for ( src = 0; src < _n_sources; src++ ) {
            _filterbank->reset();
            _filterbank->process(equalized[src], freq_bands[src], (unsigned long)_ir_length);
        }
        
        for ( src = 0; src < _n_sources; src++ ) {
            for ( idx = 0; idx < _ir_length; idx++ ) {
                results[src][idx] += freq_bands[src][band][idx];
            }
        }
    }
    
    char file_name[50];
    
    if ( write_wavs )
    {
        for ( unsigned src = 0; src < _n_sources; src++ )
        {
            sprintf( file_name, "results_%i.wav", src );
            audio_file = sf_open( file_name, SFM_WRITE, &audio_format );
            sf_write_float( audio_file, equalized[src], _ir_length );
            sf_close( audio_file );
        }
    }
    
    // Clean up.
    for ( unsigned src = 0; src < _n_sources; src++ ) {
        delete [] (equalized[src]-_max_padding);
        delete [] results[src];
    }
    delete [] equalized;
    delete [] results;
    
    
    for ( src = 0; src < _n_sources; src++ ) {
        for ( band = 0; band < _n_bands; band++ ) {
            delete [] freq_bands[src][band];
            
        }
        delete [] freq_bands[src];

    }
    delete [] freq_bands;
}

void SSRverb::Randomizer::_distribute_samples()
{
    std::mt19937 generator( time(0) );
    std::uniform_real_distribution<float> az_dist(0, 2.f*M_PI);
    std::uniform_real_distribution<float> el_dist( -M_PI, M_PI );
    
    float azimuth, elevation;
    unsigned src, selected_src;
    
    float _max_diff = 2.f * M_PI / _n_sources;
    
    for ( long long idx = 0; idx < _ir_length; idx++ ) {
        // Create random angles for direction of arrival.
        azimuth = az_dist( generator );
        elevation = el_dist( generator );
        
        // Find source with closest azimuth.
        selected_src = 0;
        for ( src = 0; src < _n_sources; src++ ) {
            if ( fabsf( _source_angles[src] - azimuth ) < _max_diff )
            {
                selected_src = src;
                break;
            }
        }
        
        // Assign impulse response value weighted with cos of elevation.
        _spac_imp_resps[selected_src][idx] = _mono_imp_resp[idx] * fabsf(cosf(elevation));
        
    }
}


// This is mayham!
void SSRverb::Randomizer::_energy_equalization( float** equalized, long long win_size )
{
    float low_limit = std::numeric_limits<float>::min() * 2;
    long long idx, win, src;
    
    laproque::FFThelper fft( (unsigned(win_size+1)) );
    long long spectr_size = fft.get_spetrum_size();
    
    // Setup sin^2 fade function;
    float win_func[win_size];
    const float phase_increment = M_PI / (win_size-1);
    float phase = 0;
    for ( idx = 0; idx < win_size; idx++) {
        win_func[idx] = powf( sinf(phase), 2 );
        //printf("%.7e\n", win_func[idx]);
        phase += phase_increment;
    }
    
    const long long step_size = floor(win_size / 2);
    const long long n_windows = ceil( float(_ir_length) / float(step_size) ) + 1;
    
    printf("Equalizing... win_size: %04lli\tstep_size: %04lli\tn_wins: %04lli\n", win_size, step_size, n_windows );
    
    // Set result array to 0 because result is summed up later.
    for ( src = 0; src < _n_sources; src++ ) {
        for ( idx = 0; idx < win_size; idx++ ) {
            equalized[src][idx] = 0.f;
        }
    }
    
    // Reserver memory for current windows and fft results
    // Normalization values for FFT results.
    float norm_factors[spectr_size];
    
    // Mono
    float mono_window[win_size+1];
    mono_window[win_size] = 0.f;
    fftwf_complex mono_fft[spectr_size];
    
    // Spacial
    float* current_windows[_n_sources];
    fftwf_complex* fft_results[_n_sources];
    for ( src = 0; src < _n_sources; src++ ) {
        current_windows[src] = new float[win_size+1];
        current_windows[src][win_size] = 0.f;
        fft_results[src] = new fftwf_complex[spectr_size];
    }
    
    // Move pointer of impulse responses to start of first window.
    _mono_imp_resp -= step_size;
    for ( src = 0; src < _n_sources; src++ ) {
        _spac_imp_resps[src] -= step_size;
        equalized[src] -= step_size;
    }
    
    // CORE Loop through all windows
    for ( win = 0; win < n_windows; win++ ) {
        // Reset norm_factors.
        for ( idx = 0; idx < spectr_size; idx++ ) {
            norm_factors[idx] = 0.f;
        }
        
        // Cut out windows
        // Mono
        for ( idx = 0; idx < win_size; idx++ ) {
            mono_window[idx] = _mono_imp_resp[idx] * win_func[idx];
        }
        
        //Spacial
        for ( src = 0; src < _n_sources; src++ ) {
            for ( idx = 0; idx < win_size; idx++ ) {
                current_windows[src][idx] = _spac_imp_resps[src][idx] * win_func[idx];
            }
        }
        
        // Compute FFTs.
        fft.real2complex( mono_window, mono_fft );
        for ( src = 0; src < _n_sources; src++ ) {
            fft.real2complex( current_windows[src], fft_results[src] );
            // Sum up energy.
            for ( idx = 0; idx < spectr_size; idx++ ) {
                norm_factors[idx] += powf( complex_abs( fft_results[src][idx] ), 2 );
            }
        }
        
        // Calculate the actual norm factors and apply them
        for ( idx = 0; idx < spectr_size; idx++ ) {
            norm_factors[idx] = sqrtf( powf( complex_abs( mono_fft[idx] ), 2 ) / (norm_factors[idx] + low_limit) / _n_sources );
            // Apply.
            for ( src = 0; src < _n_sources; src++ ) {
                fft_results[src][idx][0] *= norm_factors[idx];
                fft_results[src][idx][1] *= norm_factors[idx];
            }
        }
        
        // Back to time domain
        for ( src = 0; src < _n_sources; src++ ) {
            fft.complex2real( fft_results[src], current_windows[src] );
        }
        
        // Write results to output array.
        for ( src = 0; src < _n_sources; src++ ) {
            for ( idx = 0; idx < win_size; idx++ ) {
                equalized[src][idx] += current_windows[src][idx];
            }
        }
        
        // Increment impulse response pointers by step size.
        _mono_imp_resp += step_size;
        for ( src = 0; src < _n_sources; src++ ) {
            _spac_imp_resps[src] += step_size;
            equalized[src] += step_size;
        }
    }
    
    // Clean up.
    for ( src = 0; src < _n_sources; src++ ) {
        delete [] current_windows[src];
        delete [] fft_results[src];
    }
    
    // Set impulse responses and result pointers back to original state.
    _mono_imp_resp -= (n_windows-1) * step_size;
    for ( src = 0; src < _n_sources; src++ ) {
        _spac_imp_resps[src] -= (n_windows-1) * step_size;
        equalized[src] -= (n_windows-1) * step_size;
    }
}

std::vector<long long> SSRverb::Randomizer::_find_peaks( float *data_start, float *data_end, float threshold, long long n_peaks )
{
    std::vector<long long> results;
    long long n_found = 0;
    
    float maximum = *std::max_element( data_start, data_end );
    
    threshold *= maximum;
    
    if ( data_start[0] > threshold ) {
        if ( data_start[0] > data_start[1] ) {
            results.push_back( 0 );
            n_found++;
        }
    }
    
    // If n_peaks unspecified run through entire data.
    if ( n_peaks == 0 ) {
        for ( long long idx = 1; idx < data_end-data_start-1; idx++) {
            if ( data_start[idx] > threshold ) {
                if ( data_start[idx] > data_start[idx-1] && data_start[idx] > data_start[idx+1] ) {
                    printf("%f < %f > %f\n", data_start[idx-1], data_start[idx], data_start[idx+1]);
                    results.push_back( idx );
                }
            }
        }
    }
    
    // Else count peaks.
    else {
        for ( long long idx = 1; idx < data_end-data_start-1; idx++) {
            if ( data_start[idx] > threshold ) {
                if ( data_start[idx] > data_start[idx-1] && data_start[idx] > data_start[idx+1] ) {
                    results.push_back( idx );
                    n_found++;
                }
            }
            if ( n_found == n_peaks ) {
                break;
            }
        }
    }
    
    return results;
}

void SSRverb::Randomizer::_remove_direct()
{
    const long long fade_length = round(0.01*_sample_rate);
    const float phase_increment = 0.5f*M_PI / fade_length;
    long long idx;
    
    float abs_ir[_ir_length];
    
    for ( idx = 0; idx < _ir_length; idx++ ) {
        abs_ir[idx] = fabsf( _mono_imp_resp[idx] );
    }
    
    auto peaks = _find_peaks( abs_ir, abs_ir+_ir_length, 0.333f, 1 );
    printf( "Direct peak detected at: %lli = %.5f ms\n", peaks[0], peaks[0] / float(_sample_rate)  );
    
    // Set zeros until direct.
    for ( idx = 0; idx < peaks[0]; idx++ ) {
        _mono_imp_resp[idx] = 0.f;
    }
    
    // Sin^2 fade from there on.
    float phase = 0.f;
    for ( idx = peaks[0]; idx < peaks[0]+fade_length; idx++ ) {
        _mono_imp_resp[idx] *= powf(sinf(phase), 2.f);
        phase += phase_increment;
    }
    
}

float** SSRverb::Randomizer::get_spac_imp_resps()
{
    return _spac_imp_resps;
}

long long SSRverb::Randomizer::get_ir_length()
{
    return _ir_length;
}

