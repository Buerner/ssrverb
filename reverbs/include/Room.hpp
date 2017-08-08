//
//  Room.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef Room_hpp
#define Room_hpp

#include "Room.hpp"
#include "Plane3D.hpp"
#include "Vector3D.hpp"

#include <vector>
#include <cstdlib>
#include <fstream>

namespace SSRverb {

class Room
{
public:
    typedef Vector3D** Plane;
    typedef Vector3D*** MirroedSources;
    
    const Vector3D x_axis;
    const Vector3D y_axis;
    const Vector3D z_axis;
    
    const static unsigned N_WALLS = 6;
    
    Room( float x_size, float y_size, float z_size );
    ~Room();
    
    Plane3D* operator[] (int i);
    
    /**
     @returns Number of resulting mirror sources in case of a given order.
     */
    static unsigned get_n_mirr_src( unsigned order )
    {
        if ( order == 0 ) {
            return 0;
        }
        else {
            unsigned result = 6;
            
            for (unsigned idx = 2; idx <= order; idx++) {
                result += srcs_per_plane(idx) + srcs_per_plane(idx-1) + 2;
            }
            
            return result;
        }
    
    };
    
    static unsigned srcs_per_plane( unsigned order )
    {
        unsigned result = 0;
        
        for (unsigned idx = 1; idx <= order; idx++) {
            result += idx * 4;
        }
        
        return result;
    }
    
    static void dispose_mirror_vector( MirroedSources results, int order )
    {
        unsigned n_steps = 2*order + 1;
        
        for ( unsigned x_idx = 0; x_idx < n_steps; x_idx++ ) {
            for ( unsigned y_idx = 0; y_idx < n_steps; y_idx++ ) {
                delete [] results[x_idx][y_idx];
            }
            delete [] results[x_idx];
        }
        delete [] results;
    }
    
    static MirroedSources prepare_mirror_vector( int order )
    {
        unsigned n_steps = 2*order + 1;
        MirroedSources results = new Vector3D**[n_steps];
        for ( unsigned x_idx = 0; x_idx < n_steps; x_idx++ )
        {
            results[x_idx] = new Vector3D*[n_steps];
            for ( unsigned y_idx = 0; y_idx < n_steps; y_idx++ )
            {
                results[x_idx][y_idx] = new Vector3D[n_steps];
            }
        }
        
        return results;
    }
    
    /**
    @brief Returns the points of a given reflection order.
    @param extract The order to be extracted. Must be <= order of mirrored.
    @param mirrored Points mirrored by mirror_point().
    @param order Order of reflection in mirrored.
    @param results Array the points of the extracted order are stored in.
    */
    static void extract_order( int extract, MirroedSources mirrored, int order, Vector3D* results )
    {
        int plane, row, col;
        unsigned counter = 0;
        
        for ( plane = -extract; plane <= extract; plane++ ) {
            for ( row = abs(plane)-extract; row <= extract-abs(plane); row++ ) {
                for ( col = abs(plane)+abs(row)-extract ; col <= extract-abs(plane)-abs(row); col++) {
                    if ( abs(plane) + abs(col) + abs(row) < extract) {
                        continue;
                    }
                    //printf("[%i, %i, %i]\n", plane+order, row+order, col+order);
                    results[counter] = mirrored[plane+order][row+order][col+order];
                    counter++;
                }
            }
        }
    }

    static void write_mirror_sources( MirroedSources sources, unsigned order )
    {
        std::ofstream src_file;
        src_file.open("mirror_sources.txt");
        
        unsigned src, ord;
        
        Vector3D* this_order = new Vector3D[ get_n_mirr_src(order) - get_n_mirr_src(order-1) ];
        unsigned n_sources;
        for ( ord = 1; ord <= order; ord++ ) {
            n_sources = get_n_mirr_src( ord ) - get_n_mirr_src( ord-1 );
            
            extract_order(ord, sources, order, this_order);
            for ( src = 0; src < n_sources; src++) {
                src_file << this_order[src].get_x() << "\t" << this_order[src].get_y() << "\t" << this_order[src].get_z() << std::endl;
            }
            src_file << std::endl;
        }
        src_file.close();
        delete [] this_order;
    }
    
    /**
    @brief Mirrors a point in all walls of the room.
    @param point The point to be mirrored.
    @param order The order of reflections to be calulated.
    @param results Array the resulting points are written to. Create with prepare_mirror_vector(int order).
    */
    void mirror_point( Vector3D point,  unsigned order, MirroedSources results );
    
    /**
    @brief Calculates the length of the segments connecting a point and an observer through the reflective walls.
    @param point The point to be mirrored.
    @param observer Observer looking at the point through the mirror images.
    @param order The order of reflections to be calulated.
    @param distances Array the resulting distances are stored in.
    */
    void mirror_distances( Vector3D point, Vector3D observer, unsigned order, float* distances );
    
    /** @brief Changes size of the room in x, y and z-dimension. */
    void set_dimensions( float x, float y, float z );
    
    /** @returns Room size in x-dimension. */
    float get_x_size();
    /** @returns Room size in y-dimension. */
    float get_y_size();
    /** @returns Room size in z-dimension. */
    float get_z_size();
    
    /** @returns The volume of the room. */
    float get_volume();
    /** @returns The sum of the surface area of all walls. */
    float get_surface();
    
private:
    float _x_size;
    float _y_size;
    float _z_size;
    
    Plane3D* _walls[N_WALLS];
    
    void _setup_walls();
    void _tare_down_walls();
};

} // namespace SSRverb

#endif /* Room_hpp */
