//
//  DynamicFDN.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef DynamicFDN_hpp
#define DynamicFDN_hpp

#include <stdio.h>
#include <atomic>

#include "reverbs/include/ReverbBase.hpp"
#include "reverbs/fdnverb/include/FDN.hpp"
#include "reverbs/ismverb/include/ISMverb.hpp"

namespace SSRverb {

class DynamicFDN : public SSRverb::ReverbBase
{
public:
    DynamicFDN(  unsigned n_rev_sources = 8 );
    ~DynamicFDN();
    
    bool connect();
    
    void set_src_pos( Vector3D new_pos );
    void set_rec_pos( Vector3D new_pos );

    void set_tracked_source( unsigned source_id, float x, float y );
    
    void set_gain( float new_gain );
    void set_fdn_ism_mix( float new_mix );
    void set_ism_gain( float new_gain );
    void set_fdn_gain( float new_gain );
    
    void set_t60( float t60_value, unsigned band_idx );
    void set_co_freqs( std::vector< float > co_freqs );
    void set_room_size( float x, float y, float z );
    
    void set_tracking( bool status );
    bool get_tracking();
    
    void render_audio(
                      jack_nframes_t n_frames
                      , laproque::sample_t **in_buffers
                      , laproque::sample_t **out_buffers
                        );
    
private:
       
    FDN _fdn;
    ISMverb _ism;
    
    float** _internal_buffers;
    
    unsigned _n_remaining;
    unsigned _n_ready;
    
    static const unsigned _n_bands = 3;
    float _t60_times[_n_bands];
    
    std::atomic<float> _fdn_ism_mix{0.5f};
    std::atomic<float> _ism_gain{1.0f};
    std::atomic<float> _fdn_gain{1.0f};
    std::atomic<float> _gain{1.0f};
};

} // SSRverb namspace

#endif /* DynamicFDN_hpp */
