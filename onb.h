//
// Created by binjabin on 8/5/25.
//

#ifndef ONB_H
#define ONB_H
#include "vec3.h"


class onb {
public:
    onb( const vec3& n ) {
        //onb's z is out input (normalized)
        axis[2] = unit_vector( n );
        //select an axis not parallel with onb's z to create a plane with
        vec3 a = ( std::fabs( axis[2].x()  ) > 0.9 ) ? vec3( 0, 1, 0 ) : vec3( 1, 0, 0 );
        //get right angle vector to plane
        axis[1] = unit_vector( cross( axis[2], a ) );
        //get last axis
        axis[0] = cross( axis[2], axis[1] );
    }

    const vec3& u() const { return axis[0]; }
    const vec3& v() const { return axis[1]; }
    const vec3& w() const { return axis[2]; }

    vec3 transform( const vec3& v ) const {
        //each axis[] is a vector. Local space => World Space
        return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
    }

private:
    vec3 axis[3];
};


#endif //ONB_H
