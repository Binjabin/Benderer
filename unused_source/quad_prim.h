//
// Created by binjabin on 7/8/25.
//

#ifndef QUAD_H
#define QUAD_H

#include "../source/scene/hittables/surfaces/primitive_surface.h"
#include "../source/scene/hittables/hittable.h"
#include "../source/scene/shapes/flats/quad.h"

class quad_prim : public primitive_surface {
public:

    quad_prim( const vec3& u, const vec3& v, shared_ptr<material> mat )
        : material(mat) {

        auto n = cross( u, v );
        normal = unit_vector( n );
        D = dot( normal, Q );
        w = n / dot( n, n );

        set_bounding_box();
    }


    vec3 get_normal(point3 p) const override {
        return normal;
    }

private:

    shared_ptr<material> material;
    point3 O;
    quad q;
    aabb bbox;
};

inline shared_ptr<hittable_list> box( const point3& a, const point3& b, shared_ptr<material> mat ) {
    auto sides = make_shared<hittable_list>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    auto min = point3( std::fmin( a.x(), b.x() ), std::fmin( a.y(), b.y() ), std::fmin( a.z(), b.z() ) );
    auto max = point3( std::fmax( a.x(), b.x() ), std::fmax( a.y(), b.y() ), std::fmax( a.z(), b.z() ) );

    auto dx = vec3( max.x() - min.x(), 0, 0 );
    auto dy = vec3( 0, max.y() - min.y(), 0 );
    auto dz = vec3( 0, 0, max.z() - min.z() );

    sides->add( make_shared<quad_prim>( point3( min.x(), min.y(), max.z() ), dx, dy, mat ) ); // front
    sides->add( make_shared<quad_prim>( point3( max.x(), min.y(), max.z() ), -dz, dy, mat ) ); // right
    sides->add( make_shared<quad_prim>( point3( max.x(), min.y(), min.z() ), -dx, dy, mat ) ); // back
    sides->add( make_shared<quad_prim>( point3( min.x(), min.y(), min.z() ), dz, dy, mat ) ); // left
    sides->add( make_shared<quad_prim>( point3( min.x(), max.y(), max.z() ), dx, -dz, mat ) ); // top
    sides->add( make_shared<quad_prim>( point3( min.x(), min.y(), min.z() ), dx, dz, mat ) ); // bottom

    return sides;
}


#endif //QUAD_H
