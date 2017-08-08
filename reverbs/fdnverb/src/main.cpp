//
//  main.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

//#include "DynamicFDN.hpp"
#include "laproc/include/JackPlugin.hpp"
#include "FDN.hpp"
#include "DynamicFDN.hpp"
#include "ssrface/include/SceneManager.hpp"
#include <iostream>
#include "tools.h"

class JackFDN : public JackPlugin, public SSRverb::FDN
{
public:
    JackFDN() : JackPlugin("FDN", 1, 8 ), FDN(JackPlugin::_sample_rate, 24, 8) {
        FDN::set_boundries( 4, 6, 3 );
        FDN::set_t60( 3, 0 );
        FDN::set_t60( 2, 1 );
        FDN::set_t60( 1, 2 );
        FDN::set_co_freqs(std::vector<float>{ 300, 3000 });
    };
    
    void render_audio(
                                  jack_nframes_t n_frames
                                  , jack_sample **in_buffers
                                  , jack_sample **out_buffers
                                  )
    {
        FDN::process( in_buffers[0],  out_buffers, n_frames );
    };
};

int main()
{
    SSRverb::DynamicFDN reverb;
//    reverb.set_ssr_address("127.0.0.1", 4711);
//    reverb.connect();
//    reverb.clear_scene();
//    reverb.run();
//    reverb.setup_rev_sources();
//    usleep(100000);
    
    reverb.set_fdn_gain( 1.f );
    reverb.set_ism_gain( 1.f );
    reverb.set_room_size( 16.f, 29.f, 6.f );
    
    reverb.set_src_pos( Vector3D{3.5f, 4.5f, 1.78f} );
    reverb.set_rec_pos( Vector3D{2.5f, 2.5f, 1.78f} );
    
    reverb.set_t60( 5, 0 );
    reverb.set_t60( 3, 1 );
    reverb.set_t60( 0.8, 2 );
    
    reverb.write_imp_resp(3*44100);

    //reverb.write_imp_resp(4*44100);
    
    
//    usleep(100000);
//
//    foo.remove_rev_sources();
//
//    auto ids = foo.get_scene()->get_source_ids();
//    for (unsigned idx = 0; idx < ids.size(); idx++) {
//        printf("%s\n", foo.get_scene()->get_source(ids[idx])->name.c_str() );
//    }
//    
//    foo.disconnect();
//    foo.deactivate();
//    
//    foo.connect();
//    foo.run();
//    
//    foo.setup_rev_sources();
//    
//    usleep(100000);
//    
//    ids = foo.get_scene()->get_source_ids();
//    for (unsigned idx = 0; idx < ids.size(); idx++) {
//        printf("%s\n", foo.get_scene()->get_source(ids[idx])->name.c_str() );
//    }
//    
//    foo.remove_rev_sources();
//    
//    foo.disconnect();
//    foo.deactivate();
    
    
    return 0;
    
}
