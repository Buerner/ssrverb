//
//  Vector3D.hpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef Vector3D_hpp
#define Vector3D_hpp

namespace SSRverb {

class Vector3D
{
public:
    Vector3D( float x = 0.f, float y=0.f, float z=0.f );
    
    Vector3D operator- ( Vector3D other );
    Vector3D operator+ ( Vector3D other );
    Vector3D operator* ( float factor );
    Vector3D operator* ( Vector3D other );
    Vector3D operator/ ( float divisor );
    Vector3D operator/ ( float divisor ) const;
    void operator/= ( float divisor );
    void operator= ( Vector3D substitute );
    float operator[] (int i);
    float operator[] (int i) const;
    
    // Skalarprodukt
    float operator% ( Vector3D other );
    
    Vector3D get_unit();
    
    float get_length();
    
    float distance_to( Vector3D other );
    //float angle_to( Vector3D other );
    
    float azimuth_to( Vector3D other);
    
    float get_x();
    float get_y();
    float get_z();
    
    float get_azimuth();
    
    void set_x( float x );
    void set_y( float y );
    void set_z( float z );
    
protected:
    float _x, _y, _z;
    
};

} // namespace SSRverb

#endif /* Vector3D_hpp */
