//
// Created by binjabin on 6/20/25.
//

#ifndef RAY_H
#define RAY_H
#include "vec3.h"


class ray {
public:
    ray() {
    }

    ray( const point3& origin, const vec3& direction, double time )
        : orig( origin ), dir( direction ), tm( time ) {
        t_len = direction.length();
        inv_t_len = (t_len > 0) ? 1.0 / t_len : 0.0;
    }

    ray( const point3& origin, const vec3& direction )
        : ray( origin, direction, 0 ) {
    }

    const point3& origin() const { return orig; }
    const vec3& direction() const { return dir; }

    double time() const { return tm; }

    point3 at( double t ) const {
        return orig + t * dir;
    }

    //Convert a ray param t into a global distance (and vice versa)
    double t_to_distance(double t) const { return t * t_len; }
    double distance_to_t(double dist) const {return dist * inv_t_len; }


private:
    point3 orig;
    vec3 dir;
    double tm;
    double t_len;
    double inv_t_len;
};


#endif //RAY_H
