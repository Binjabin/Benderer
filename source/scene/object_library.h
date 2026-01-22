//
// Created by binjabin on 1/22/26.
//

#ifndef BENDERER_OBJECT_LIBRARY_H
#define BENDERER_OBJECT_LIBRARY_H

#include "hittables/hittable.h"
#include "hittables/hittable_list.h"
#include "hittables/primitive.h"
#include "hittables/transformed.h"
#include "material/material.h"

#include "transforms/translate.h"
#include "transforms/rotate_y.h"

#include "shapes/flats/quad.h"
#include "shapes/solids/sphere.h"

namespace object_library {

    inline shared_ptr<hittable> make_sphere( const point3& o, double r, shared_ptr<material> mat ) {
        auto ball = make_shared<primitive>(make_shared<sphere>(r), mat);
        return make_shared<transformed>(ball, make_shared<translate>(o));
    }

    inline shared_ptr<hittable> make_quad(const point3& o, const vec3& u, const vec3& v, shared_ptr<material> mat) {
        auto ball = make_shared<primitive>(make_shared<quad>(u, v), mat);
        return make_shared<transformed>(ball, make_shared<translate>(o));
    }

    inline shared_ptr<hittable> make_box(const point3& a, const point3& b, shared_ptr<material> mat) {
        auto sides = make_shared<hittable_list>();

        // Construct the two opposite vertices with the minimum and maximum coordinates.
        auto min = point3( std::fmin( a.x(), b.x() ), std::fmin( a.y(), b.y() ), std::fmin( a.z(), b.z() ) );
        auto max = point3( std::fmax( a.x(), b.x() ), std::fmax( a.y(), b.y() ), std::fmax( a.z(), b.z() ) );

        auto dx = vec3( max.x() - min.x(), 0, 0 );
        auto dy = vec3( 0, max.y() - min.y(), 0 );
        auto dz = vec3( 0, 0, max.z() - min.z() );

        sides->add( make_quad( point3( min.x(), min.y(), max.z() ), dx, dy, mat ) ); // front
        sides->add( make_quad( point3( max.x(), min.y(), max.z() ), -dz, dy, mat ) ); // right
        sides->add( make_quad( point3( max.x(), min.y(), min.z() ), -dx, dy, mat ) ); // back
        sides->add( make_quad( point3( min.x(), min.y(), min.z() ), dz, dy, mat ) ); // left
        sides->add( make_quad( point3( min.x(), max.y(), max.z() ), dx, -dz, mat ) ); // top
        sides->add( make_quad( point3( min.x(), min.y(), min.z() ), dx, dz, mat ) ); // bottom

        return sides;
    }

    inline shared_ptr<hittable> make_rotate(shared_ptr<hittable> obj, double theta) {
        return make_shared<transformed>(obj, make_shared<rotate_y>(theta));
    }

    inline shared_ptr<hittable> make_translate(shared_ptr<hittable> obj, const vec3& offset) {
        return make_shared<transformed>(obj, make_shared<translate>(offset));
    }

}

#endif //BENDERER_OBJECT_LIBRARY_H