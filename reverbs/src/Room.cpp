//
//  Room.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include <algorithm>
#include <math.h>

#include "Room.hpp"


SSRverb::Room::Room( float x_size, float y_size, float z_size ) :
    _x_size(x_size), _y_size(y_size), _z_size(z_size),
    x_axis(1.f, 0.f, 0.f), y_axis(0.f, 1.f, 0.f), z_axis(0.f, 0.f, 1.f)
{
    _setup_walls();
}

SSRverb::Room::~Room()
{
    _tare_down_walls();
}

void SSRverb::Room::_setup_walls()
{
    // right wall
    _walls[0] = new Plane3D(
                            Vector3D(_x_size, 0.f, 0.f)
                            , y_axis
                            , z_axis
                            );
    // back wall
    _walls[1] = new Plane3D(
                            Vector3D(0.f, _y_size, 0.f)
                            , x_axis
                            , z_axis
                            );
    // left wall
    _walls[2] = new Plane3D(
                            Vector3D(0.f, 0.f, 0.f)
                            , y_axis
                            , z_axis
                            );
    // front wall
    _walls[3] = new Plane3D(
                            Vector3D(0.f, 0.f, 0.f)
                            , x_axis
                            , z_axis
                            );
    // ceiling
    _walls[4] = new Plane3D(
                            Vector3D(0.f, 0.f, _z_size)
                            , x_axis
                            , y_axis
                            );
    // floor
    _walls[5] = new Plane3D(
                            Vector3D(0.f, 0.f, 0.f)
                            , x_axis
                            , y_axis
                            );
}

void SSRverb::Room::_tare_down_walls()
{
    for ( unsigned idx = 0; idx < N_WALLS; idx++) {
        delete _walls[idx];
    }
}


void SSRverb::Room::mirror_point( Vector3D point,  unsigned order, MirroedSources results )
{
    // Get delta vectors
    Vector3D delta_x_plus  = _walls[0]->get_connection( point ) * 2.f;
    Vector3D delta_x_minus = _walls[2]->get_connection( point ) * 2.f;
    Vector3D delta_y_plus  = _walls[1]->get_connection( point ) * 2.f;
    Vector3D delta_y_minus = _walls[3]->get_connection( point ) * 2.f;
    Vector3D delta_z_plus  = _walls[4]->get_connection( point ) * 2.f;
    Vector3D delta_z_minus = _walls[5]->get_connection( point ) * 2.f;
    
    float offset_plus, offset_minus;
    unsigned row, col, plane, start;
    
    // Create middle plane
    Plane middle = results[order];
    unsigned n_steps = 2*order+1;
    float primary_fact, secondary_fact;
    
    // set center to source value
    middle[order][order] = point;
    
    // Adjust y
    for ( row = 0; row < order; row++) {
        primary_fact = ceil((order-row)/2.f);
        secondary_fact = floor((order-row)/2.f);
        
        offset_plus  = delta_y_plus[1] * primary_fact - delta_y_minus[1] * secondary_fact;
        offset_minus = delta_y_minus[1] * primary_fact - delta_y_plus[1]  * secondary_fact;
        
        for ( col = order - row; col < order + row + 1; col++)
        {
            middle[row][col].set_y( point[1] + offset_minus );
            middle[n_steps-row-1][col].set_y( point[1] + offset_plus );
        }
    }
    
    // Adjust x
    for ( col = 0; col < order; col++) {
        primary_fact = ceil((order-col)/2.f);
        secondary_fact = floor((order-col)/2.f);
        
        offset_plus  = delta_x_plus[0] * primary_fact - delta_x_minus[0] * secondary_fact;
        offset_minus = delta_x_minus[0] * primary_fact - delta_x_plus[0]  * secondary_fact;
        
        for ( row = order - col; row < order + col + 1; row++)
        {
            middle[row][col].set_x( point[0] + offset_minus );
            middle[row][n_steps-col-1].set_x( point[0] + offset_plus );
        }
    }
    
    // Handle middle row/col x/y values
    for ( col = 0; col < n_steps; col++) {
        middle[order][col].set_y( point[1] );
        middle[col][order].set_x( point[0] );
    }
    
    // Set z in middle plane
    for ( row = 0; row < n_steps ; row++) {
        start = int(fabs(float(order)-row));
        for ( col =  start; col <  n_steps-start; col++)
        {
            //printf("Plane %i: [%i, %i]\n", plane, row, col);
            middle[row][col].set_z( point[2] );
            middle[n_steps-row-1][col].set_z( point[2] );
        }
    }
    
    // Mirror subparts of middle plane in both z directions
    Vector3D offset_vec_plus;
    Vector3D offset_vec_minus;
    
    for ( plane = 0; plane < order; plane++ )
    {
        primary_fact = ceil((order-plane)/2.f);
        secondary_fact = floor((order-plane)/2.f);
        
        offset_vec_plus  = delta_z_plus * primary_fact - delta_z_minus * secondary_fact;
        offset_vec_minus = delta_z_minus * primary_fact - delta_z_plus  * secondary_fact;
        
        for ( row = order-plane; row < order+plane+1 ; row++) {
            start = int(fabs(float(order)-row)+(order-plane));
            for ( col =  start; col <  n_steps-start; col++)
            {
                //printf("Plane %i: [%i, %i]\n", plane, row, col);
                results[plane][row][col] = middle[row][col] + offset_vec_minus ;
                results[n_steps-plane-1][row][col] = middle[row][col] + offset_vec_plus ;
            }
        }
    }
}

void SSRverb::Room::mirror_distances( Vector3D point, Vector3D receiver, unsigned int order, float* distances )
{
    MirroedSources mirrors = prepare_mirror_vector( order );

    Vector3D* this_order = new Vector3D[ get_n_mirr_src(order) ];
    unsigned n_sources;
    for ( unsigned ord = 1; ord <= order; ord++ ) {
         n_sources = get_n_mirr_src( order );
        
        extract_order(ord, mirrors, order, this_order);
        for ( unsigned src = 0; src < n_sources; src++) {
            *distances++ = this_order[src].distance_to( receiver );
        }
        
    }

    delete [] this_order;
    dispose_mirror_vector( mirrors, order );
}

void SSRverb::Room::set_dimensions(float x, float y, float z)
{
    _tare_down_walls();
    
    _x_size = x;
    _y_size = y;
    _z_size = z;
    
    _setup_walls();
}

float SSRverb::Room::get_x_size()
{
    return _x_size;
}

float SSRverb::Room::get_y_size()
{
    return _y_size;
}

float SSRverb::Room::get_z_size()
{
    return _z_size;
}

SSRverb::Plane3D* SSRverb::Room::operator[] ( int i )
{
    if (i < N_WALLS) return _walls[i];
    else return nullptr;
    
}

float SSRverb::Room::get_volume()
{
    return _x_size * _y_size * _z_size;
}

float SSRverb::Room::get_surface()
{
    return 2.f*(_x_size * _y_size) + 2.f*(_x_size * _z_size) + 2.f*(_y_size * _z_size);
}

