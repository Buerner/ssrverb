//
//  ReverbBase.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef ReverbBase_hpp
#define ReverbBase_hpp

#include <stdio.h>
#include <algorithm>
#include <mutex>
#include <atomic>

#include "Vector3D.hpp"
#include "laproque/include/JackPlugin.hpp"
#include "ssrface/include/SceneManager.hpp"

namespace SSRverb {

class ReverbBase : public laproque::JackPlugin, public ssrface::SceneManager
{
public:
    ReverbBase(   const char* name
                , unsigned n_rev_sources = 8
              );
    
    ~ReverbBase();
    
    /**
    @brief This is the audio processing callback function. 
    @param n_frames Number of frames to be processed in this block.
    @param in_buffers Arrays contining an array with samples for each input.
    @param out_buffers Arrays contining an array with samples for each output.
    */
    virtual void render_audio(  laproque::nframes_t n_frames
                              , laproque::sample_t **in_buffers
                              , laproque::sample_t **out_buffers
                              ) = 0;
    
    /**
    @brief Creates the sources used for reverberation in the SSR.

    The SceneManager functionalities are used to set up reverberation sources 
    uniformly spaced on a circle in the SSR. This of course only works in case
    a connection was successfully established.
    */
    void setup_rev_sources();

    /**
    @brief Try to connect to SSRs TCP/IP interface.
    */
    void connect_to_ssr();

    /**
    @brief Remove the reverberation sources.

    Can only be done when a connection to the SSR is established and in case
    the reverberation sources have been tracked correctly.
    (Once issue #45 in SSR is fixed.)
    */
    void remove_rev_sources();
    
    /** @brief Changes the position of the internal virtual sound source. */
    virtual void set_src_pos( Vector3D new_pos );
    /** @brief Changes the position of the internal virtual receiver. */
    virtual void set_rec_pos( Vector3D new_pos );
    
    /** @brief Set the ID of the source to be tracked in the SSR. */
    virtual void set_tracked_source( unsigned source_id );

    /** @brief Change the distance of all reverberation sources.  */
    void set_radius( float new_radius );
    
    /** @returns Vector with the SSR IDs of the reverberation sources. */
    std::vector< unsigned short > get_rev_ids();
    
    /** @brief Stops SceneManager update process and deactivats JACK client. */
    void end();
    
    /**
    @brief Checks if a source is tracked as reverberation sources.
    @param id SSR source ID you want to check. 
    */
    bool is_rev_source( unsigned short id );
    
    // Called when on new source in SSR.
    static void track_rev_sources( ssrface::Scene* scene_ptr, void* io_data )
    {
        SSRverb::ReverbBase* rev_base = (SSRverb::ReverbBase*)io_data;
        std::vector< unsigned short > ids = scene_ptr->get_source_ids();
        
        ssrface::Source* source;
        
        // Loop through source map.
        for ( unsigned idx = 0; idx < ids.size(); idx++ )
        {
            source = scene_ptr->get_source( ids[idx] );
            if ( source == nullptr ) continue;
            
            // Check if name prefix fits.
            if ( !strncmp( source->name.c_str(), rev_base->_rev_name, rev_base->_prefix_length ) )
            {
                // Check if ID is already in list.
                if ( std::find( rev_base->_rev_source_ids.begin(), rev_base->_rev_source_ids.end(), source->id ) != rev_base->_rev_source_ids.end() ) {
                    continue;
                }
                else {
                    rev_base->_rev_source_ids.push_back( source->id );
                }
            }
        }
    };
    
    /** @brief Callback function for SceneManager which trackes the SSR reference position. */
    static void track_reference( ssrface::Scene* scene_ptr, void* io_data )
    {
        SSRverb::ReverbBase* ReverbBase = (SSRverb::ReverbBase*)io_data;
        
        ssrface::Source* ref = scene_ptr->get_reference();
        
        ReverbBase->_rec_pos = Vector3D{ref->x, ref->y, ReverbBase->_rec_pos[2]};

        if ( ReverbBase->_rev_srcs_set.load() ) ReverbBase->_update_rev_sources();
    };
    
protected:
    unsigned _n_rev_sources;
    std::vector<unsigned short> _rev_source_ids;
    float _radius = 1.f;
    
    std::atomic<bool> _rev_srcs_set{ false };
    
    Vector3D _src_pos;
    Vector3D _rec_pos;
    
    unsigned _tracked_source_id;
    
    const char* _rev_name = "rev_";
    const unsigned _prefix_length = 4;
    
    void _update_rev_sources();
    
    std::mutex _mtx;
    
};

} // SSRverb namspace

#endif /* ReverbBase_hpp */
