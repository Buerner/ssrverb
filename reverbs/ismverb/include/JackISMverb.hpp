//
//  JackISMverb.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef JackISMverb_hpp
#define JackISMverb_hpp

#include "laproque/include/JackPlugin.hpp"
#include "ISMverb.hpp"
#include "reverbs/include/ReverbBase.hpp"

namespace SSRverb {

class JackISMverb : public SSRverb::ReverbBase
{
public:
    JackISMverb( float x, float y, float z, unsigned order );
    ~JackISMverb();
    
    void render_audio( laproque::nframes_t n_frames, laproque::sample_t **in_buffers, laproque::sample_t **out_buffers );
    
    void activate();
    
    void set_src_pos( Vector3D new_pos );
    void set_rec_pos( Vector3D new_pos );
    void set_tracked_source( unsigned id );
    
private:
    SSRverb::ISMverb _ism;

    
};

} // namespace SSRverb

#endif /* JackISMverb_hpp */
