//
//  JackRandomizer.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef JackRandomizer_hpp
#define JackRandomizer_hpp

#include "reverbs/include/ReverbBase.hpp"
#include "laproque/include/Convolver.hpp"
#include <array>

namespace SSRverb {

class JackRandomizer : public SSRverb::ReverbBase
{
public:
    JackRandomizer(const char* wav_path);
    ~JackRandomizer();
    const static unsigned n_convolvers = 8;
    
    void render_audio(
                      laproque::nframes_t n_frames
                      , laproque::sample_t **in_buffers
                      , laproque::sample_t **out_buffers
                      );
    
private:
    std::array<laproque::Convolver*, n_convolvers> _convolvers;
};

} // namespace SSRverb

#endif /* JackRandomizer_hpp */
