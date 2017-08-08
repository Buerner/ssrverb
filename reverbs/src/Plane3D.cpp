//
//  SSRverb::Plane3D.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "Plane3D.hpp"
#include <math.h>

SSRverb::Plane3D::Plane3D( Vector3D pos_vec, Vector3D dir_vec1, Vector3D dir_vec2 )
{
    _pos_vec = pos_vec;
    _dir_vec1 = dir_vec1.get_unit();
    _dir_vec2 = dir_vec2.get_unit();
    
    _normal = (dir_vec1 * dir_vec2);
    
    float norm_factor = 1.f / powf( _normal.get_length(), 2.f );
    
    _d = (_normal % _pos_vec) * norm_factor;

    _normal = _normal * norm_factor;
}

float SSRverb::Plane3D::get_distance( Vector3D point )
{
    return _d - (_normal % point);
}

SSRverb::Vector3D SSRverb::Plane3D::get_normal()
{
    return _normal;
}

SSRverb::Vector3D SSRverb::Plane3D::mirror( Vector3D point )
{
    Vector3D connection( _normal * (get_distance( point ) * 2) );
    
    return point + connection;
}

SSRverb::Vector3D SSRverb::Plane3D::get_connection( Vector3D point )
{
    return _normal * get_distance( point );
}


SSRverb::Vector3D SSRverb::Plane3D::get_intersect( Vector3D from, Vector3D to )
{
    float lambda = _d - _normal[0]*from[0] - _normal[1]*from[1] - _normal[2]*from[2];
    lambda /= (_normal[0]*to[0] + _normal[1]*to[1] + _normal[2]*to[2]);
    
    return from + (to*lambda);
}
