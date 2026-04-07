//
// Created by binjabin on 1/22/26.
//

#ifndef BENDERER_OBJECT_LIBRARY_H
#define BENDERER_OBJECT_LIBRARY_H

#include "hittables/hittable.h"
#include "hittables/mediums/primitive_medium.h"
#include "hittables/mediums/transformed_medium.h"
#include "hittables/surfaces/primitive_surface.h"
#include "hittables/surfaces/transformed_surface.h"
#include "material/surface_material.h"

#include "transforms/translate.h"
#include "transforms/rotate_y.h"

#include "shapes/flats/quad.h"
#include "shapes/solids/sphere.h"
#include "shapes/solids/box.h"

class object_library {
public:
    static inline shared_ptr<surface> make_sphere( const point3& o, double r, shared_ptr<surface_material> mat ) {
        auto ball = make_shared<primitive_surface>(make_shared<sphere>(r), mat);
        return make_shared<transformed_surface>(ball, make_shared<translate>(o));
    }

    static inline shared_ptr<medium> make_sphere_medium( const point3& o, double r, shared_ptr<medium_material> mat ) {
        auto ball = make_shared<primitive_medium>(make_shared<sphere>(r), mat);
        return make_shared<transformed_medium>(ball, make_shared<translate>(o));
    }

    static inline shared_ptr<medium> make_box_medium( const point3& center, const vec3& half_size, shared_ptr<medium_material> mat ) {
        auto b = make_shared<primitive_medium>(make_shared<box>(half_size), mat);
        return make_shared<transformed_medium>(b, make_shared<translate>(center));
    }

    static inline shared_ptr<surface> make_quad(const point3& o, const vec3& u, const vec3& v, shared_ptr<surface_material> mat) {
        auto ball = make_shared<primitive_surface>(make_shared<quad>(u, v), mat);
        return make_shared<transformed_surface>(ball, make_shared<translate>(o));
    }

    static inline shared_ptr<surface> make_box(const point3& a, const point3& b, shared_ptr<surface_material> mat) {
        auto sides = make_shared<surface_list>();

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

    static inline shared_ptr<surface> make_rotate(shared_ptr<surface> obj, double theta) {
        return make_shared<transformed_surface>(obj, make_shared<rotate_y>(theta));
    }

    static inline shared_ptr<surface> make_translate(shared_ptr<surface> obj, const vec3& offset) {
        return make_shared<transformed_surface>(obj, make_shared<translate>(offset));
    }

};

#endif //BENDERER_OBJECT_LIBRARY_H