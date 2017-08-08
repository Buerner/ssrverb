//
//  Plane3D.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef Plane3D_hpp
#define Plane3D_hpp

#include "Vector3D.hpp"

namespace SSRverb {

/**
@class Plane3D
Represents a 2D plane in 3D Space.
 */
class Plane3D
{
public:
    Plane3D();
    /**
    @param pos_vec Support vector of the plane.
    @param dir_vec1 First direction vector.
    @param dir_vec2 Second direction vector.
    */
    Plane3D( Vector3D pos_vec, Vector3D dir_vec1, Vector3D dir_vec2 );
    
    /** @returns Distance between plane instance and provided point. */
    float get_distance( Vector3D point );
    
    /** @returns Griven point mirroed in the plane instance.*/
    Vector3D mirror( Vector3D point );

    /** @returns Normal vector of the plane instance. */
    Vector3D get_normal();

    /** @returns Shortest vector between plane instance and provided point. */
    Vector3D get_connection( Vector3D point );
    
    /** @returns Point of intersection of segement provided by start and end vector and plane instance. */
    Vector3D get_intersect( Vector3D from, Vector3D to );
    
private:
    Vector3D _pos_vec, _dir_vec1, _dir_vec2, _normal;
    //float _parameters[3];
    float _d;
};

} // namespace SSRverb

#endif /* Plane3D_hpp */
