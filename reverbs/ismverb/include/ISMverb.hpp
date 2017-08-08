//
//  ISMverb.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef ISMverb_hpp
#define ISMverb_hpp

#include "reverbs/include/Room.hpp"
#include "laproque/include/FadingMultiDelay.hpp"
#include "laproque/include/Filterbank.hpp"
#include "ssrface/include/Scene.hpp"

#include <atomic>
#include <vector>
#include <memory>

template <typename T> int sign(T value) {
    return (T(0) < value) - (value < T(0));
}

const std::vector< float > ISM_BAND_WEIGHTS{.9f, .8f, .7f};
const std::vector<float> ISM_CO_FREQS{ 300.f, 3000.f };

namespace SSRverb {

/**
 @class ISMverb Implementation of an Image Source Model (ISM) for cuboid-shaped rooms with uniformly reflecting walls.
 **/
class ISMverb
{
public:
    /**
     @param x Size of room in x-dimension.
     @param y Size of room in y-dimension.
     @param z Size of room in z-dimension.
     @param order Reflection oder used in this ISM instance.
     @param sample_rate Sampling frequency of processed audio signal.
     @param block_size Number of samples in one audio signal block.
     */
    ISMverb(
              float x
            , float y
            , float z
            , unsigned order
            , unsigned sample_rate
            , unsigned block_size
            );
    ~ISMverb();
    
    /**
     @brief Compute ISM result of input and write to outputs channel.
     @param input Pointer to array with input samples.
     @param outputs Pointer to array with pointers to output buffers.
     @n_frames Number of audio frames to be processed.
     */
    void process( float *input, float **outputs, unsigned long n_frames );
    
    /**
     @brief Change the position of the receiver.
     @param receiver New position in 3D space.
     */
    void set_receiver( Vector3D receiver );
    
    /**
     @brief Change the position of the sound source.
     @param source New position in 3D space.
     */
    void set_source( Vector3D source );
    
    /**
     @brief Changes the dimensions of the cuboid-shaped toom.
     */
    void set_room_dimensions( float x, float y, float z );
    
    /**
     @brief Set the SSR ID of the sound source to be tracked.
     */
    void set_tracked_source( unsigned source_id );
    
    /**
     @brief Change the cutoff frequencies of the filterbanks used for frequency-dependant reflections.
     */
    void set_co_freqs( std::vector<float> co_freq );
    
    /**
     @brief Change the weight of one frequency band in the reflection process.
     @param weight The new weight value.
     @param band_idx Index of according frequency band.
     */
    void set_band_weight( float weight, unsigned band_idx );
    
    /**
     @brief Set the state of source tracking.
     */
    void set_tracking( bool status );
    
    /**
     @brief Change the reverberation time in one frequency band.
     @param value The new reverberation time value.
     @param band_idx Index of according frequency band.
     */
    void set_t60( float value, unsigned band_idx );
    
    /** @returns State of source tracking. */
    bool get_tracking();
    
    /** @returns Position of receiver. */
    Vector3D get_receiver();
    
    /** @returns Position of sound source */
    Vector3D get_source();
    
    /** @brief Callback fuction for SceneManager to track sound source position. */
    static void update_src_pos( ssrface::Scene* scene_ptr, void* ismverb_ptr )
    {
        // Cast reverb pointer.
        ISMverb* ismverb = (ISMverb*)ismverb_ptr;
        
        if ( ismverb->get_tracking() )
        {
            unsigned src_id = ismverb->_tracked_source_id;
            
            ssrface::Source* source = scene_ptr->get_source( src_id );
            
            if ( source != nullptr )
            {
                //printf( "ISM callback: Moving source to: (%f, %f)\n", source->x, source->y );
                // Create vector instance.
                Vector3D new_pos( source->x, source->y, 1.7f );
                
                ismverb->set_source( new_pos );
            }
        }
    };
    
private:
    static const unsigned _n_rev_sources = 8;
    static const unsigned _n_freq_bands = 3;
    
    unsigned _sample_rate;
    unsigned _block_size;
    unsigned _tracked_source_id = 0;
    std::atomic<bool> _tracking_active{true};
    
    std::atomic<bool> _has_changed{false};
    
    // Image source model related members
    unsigned _order;
    Room _room;
    
    Vector3D _src_pos;
    Vector3D _rec_pos;
    float _rec_orientation = 0.f;
    
    // Mirrored sources related members
    unsigned _n_mirr_sources;
    Room::MirroedSources _mirror_sources;
    unsigned* _sources_in_order;
    Vector3D* _one_order;
    
    float _rev_source_angles[_n_rev_sources];
    float _max_anglular_distance;
    
    // Audio processing related members
    laproque::FadingMultiDelay*** _delays;
    laproque::Filterbank** _filterbanks;
    
    float** _band_weights;
    
    unsigned** _delay_counters;
    unsigned long*** _delay_values;
    float*** _delay_weights;
    
    float** _band_buffers;
    float* _internal_buffer;
    
    float* _delay_output;
    
    // Functions
    void _update_delays();
    void _make_allocations();
    
    unsigned long _call_counter;
    
};

} // namespace SSRverb

#endif /* ISMverb_hpp */
