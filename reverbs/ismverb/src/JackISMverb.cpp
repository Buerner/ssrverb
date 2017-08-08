//
//  JackISMverb.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "JackISMverb.hpp"
#include <sndfile.h>

SSRverb::JackISMverb::JackISMverb( float x, float y, float z, unsigned order ) :
    ReverbBase("SSRverb::JackISMverb", 8),
    _ism(x, y, z, order, _sample_rate, _block_size)
{
    _ism.set_tracked_source(9);
    _ism.set_receiver(Vector3D{x/2.f, y/2.f, z/2.f});
    _ism.set_source(Vector3D{x/3.f, y/3.f, z/3.f});

    set_update_callback( SSRverb::ISMverb::update_src_pos, &_ism );
    
}

void SSRverb::JackISMverb::activate()
{
    JackPlugin::activate();
    run();
    connect_to_ssr();
}

SSRverb::JackISMverb::~JackISMverb()
{
    deactivate();
}

void SSRverb::JackISMverb::render_audio(
                           laproque::nframes_t n_frames
                           , laproque::sample_t **in_buffers
                           , laproque::sample_t **out_buffers
                           )
{
    _ism.process( in_buffers[0], out_buffers, n_frames );
}

void SSRverb::JackISMverb::set_src_pos( Vector3D new_pos )
{
    _ism.set_source( new_pos );
}
void SSRverb::JackISMverb::set_rec_pos( Vector3D new_pos )
{
    _ism.set_receiver( new_pos );
    ReverbBase::set_rec_pos( new_pos );
}

void SSRverb::JackISMverb::set_tracked_source( unsigned id )
{
    _ism.set_tracked_source( id );
}


