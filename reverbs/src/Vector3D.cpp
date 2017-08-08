//
//  Vector3D.cpp
//  SSRverb - https://github.com/Buerner/SSRverb
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "Vector3D.hpp"

#include <math.h>

SSRverb::Vector3D::Vector3D( float x, float y, float z )
{
    _x = x;
    _y = y;
    _z = z;
}

float SSRverb::Vector3D::get_x() { return _x; }
float SSRverb::Vector3D::get_y() { return _y; }
float SSRverb::Vector3D::get_z() { return _z; }

void SSRverb::Vector3D::set_x( float x ) { _x = x; }
void SSRverb::Vector3D::set_y( float y ) { _y = y; }
void SSRverb::Vector3D::set_z( float z ) { _z = z; }

float SSRverb::Vector3D::operator[] (int i)
{
    switch (i) {
        case 0:
            return _x;
            break;
        case 1:
            return _y;
            break;
        case 2:
            return _z;
            break;
        default:
            return 0.f;
            break;
    }
}

float SSRverb::Vector3D::operator[] (int i) const
{
    switch (i) {
        case 0:
            return _x;
            break;
        case 1:
            return _y;
            break;
        case 2:
            return _z;
            break;
        default:
            return 0.f;
            break;
    }
}

SSRverb::Vector3D SSRverb::Vector3D::operator-( SSRverb::Vector3D other )
{
    return SSRverb::Vector3D( _x - other.get_x(), _y - other.get_y(), _z - other.get_z() );
}

SSRverb::Vector3D SSRverb::Vector3D::operator+( SSRverb::Vector3D other )
{
    return SSRverb::Vector3D( _x + other.get_x(), _y + other.get_y(), _z + other.get_z() );
}

SSRverb::Vector3D SSRverb::Vector3D::operator*( float factor )
{
    return SSRverb::Vector3D( _x * factor, _y * factor, _z * factor );
}

SSRverb::Vector3D SSRverb::Vector3D::operator/( float divisor )
{
    return SSRverb::Vector3D( _x / divisor, _y / divisor, _z / divisor );
}

SSRverb::Vector3D SSRverb::Vector3D::operator/( float divisor ) const
{
    return SSRverb::Vector3D( _x / divisor, _y / divisor, _z / divisor );
}

void SSRverb::Vector3D::operator/=( float divisor )
{
    _x /= divisor;
    _y /= divisor;
    _z /= divisor;
}

void SSRverb::Vector3D::operator=( SSRverb::Vector3D substitute )
{
    _x = substitute[0];
    _y = substitute[1];
    _z = substitute[2];
}

float SSRverb::Vector3D::get_length()
{
    return sqrtf( powf(_x, 2) + powf(_y, 2) + powf(_z, 2) );
}

float SSRverb::Vector3D::distance_to( SSRverb::Vector3D other )
{
    SSRverb::Vector3D connecting = *this - other;
    return connecting.get_length();
}

float SSRverb::Vector3D::get_azimuth()
{
    float result = atan2f( _y, _x );
    return result; // - roundf( result/ (2*M_PI) ) * 2*M_PI;
}

float SSRverb::Vector3D::azimuth_to( SSRverb::Vector3D other )
{
    SSRverb::Vector3D relative = other - *this;
    return relative.get_azimuth();
}

SSRverb::Vector3D SSRverb::Vector3D::get_unit()
{
    return *this / this->get_length();
}
SSRverb::Vector3D SSRverb::Vector3D::operator*( SSRverb::Vector3D other )
{
    float x, y, z;
    
    x = this->get_y() * other.get_z() - this->get_z() * other.get_y();
    y = this->get_z() * other.get_x() - this->get_x() * other.get_z();
    z = this->get_x() * other.get_y() - this->get_y() * other.get_x();
    
    return SSRverb::Vector3D( x, y, z );
}

float SSRverb::Vector3D::operator%( SSRverb::Vector3D other )
{
    return _x * other.get_x() + _y * other.get_y() + _z * other.get_z();
}

