//
//  JackRandomizer.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "JackRandomizer.hpp"
#include "Randomizer.hpp"
#include <sndfile.h>


SSRverb::JackRandomizer::JackRandomizer(const char* wav_path) : SSRverb::ReverbBase("JackRandomizer", 8 )
{
    SNDFILE* audio_file;
    SF_INFO audio_format;
    float* imp_resp;
    
    // Open audio file.
    audio_file = sf_open( wav_path, SFM_READ, &audio_format);
    if ( !audio_file ) printf("File %s not found.\n", wav_path);
    // Read audio data.
    else {
        imp_resp = new float[audio_format.frames];
        sf_read_float( audio_file, imp_resp, audio_format.frames );
        
        Randomizer ir_randomizer(wav_path);
        ir_randomizer.create_spacial_imp_resp();
        float** spacial_irs = ir_randomizer.get_spac_imp_resps();

        for (unsigned idx = 0; idx < n_convolvers; idx++)
        {
            _convolvers[idx] = new laproque::Convolver(spacial_irs[idx], audio_format.frames, _block_size);
        }
        
        delete [] imp_resp;
    }
}

SSRverb::JackRandomizer::~JackRandomizer()
{
    for ( unsigned idx = 0; idx < n_convolvers; idx++ ) {
        delete _convolvers[idx];
    }
}

void SSRverb::JackRandomizer::render_audio(
                           laproque::nframes_t n_frames
                           , laproque::sample_t **in_buffers
                           , laproque::sample_t **out_buffers )
{
    for ( unsigned idx = 0; idx < n_convolvers; idx++ ) {
        _convolvers[idx]->process(in_buffers[0], out_buffers[idx]);
    }
}
