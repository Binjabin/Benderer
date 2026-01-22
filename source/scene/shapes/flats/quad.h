//
// Created by binjabin on 1/21/26.
//

#ifndef BENDERER_QUAD_H
#define BENDERER_QUAD_H
#include "flat.h"
#include "../shape.h"
#include "../../../records/intersection.h"

class quad : public flat {

public:

    quad(const vec3& u, const vec3& v) : u(u), v(v) {
        vec3 n_vec = cross(u, v);
        normal = unit_vector(n_vec);
        area = n_vec.length();
        w = n_vec / dot(n_vec, n_vec);
    }

    bool intersect(const ray &r, interval ray_t, intersection& isect) const override {
        auto denom = dot( normal, r.direction() );

        //no hit if ray is parallel (enough) with the plane
        if ( std::fabs( denom ) < epsilon )
            return false;

        //return false if t is outside ray range
        auto t = -dot( normal, r.origin()) / denom;
        if ( !ray_t.contains( t ) ) {
            return false;
        }
        auto isect_p = r.at( t );
        auto alpha = dot( w, cross( isect_p, v ) );
        auto beta = dot( w, cross( u, isect_p ) );

        if (alpha < 0.0f || alpha > 1.0f || beta < 0.0f || beta > 1.0f) {
            return false;
        }

        isect.set_interaction_values(t, isect_p, r.time());
        // Determine if we are hitting the front face (ray from outside)
        bool front_face = dot(r.direction(), normal) < 0;
        isect.m_front_face = front_face;
        isect.m_normal = front_face ? normal : -normal;
        isect.m_u = alpha;
        isect.m_v = beta;
        return true;
    }

    bool intersect_check(const ray &r, interval ray_t) const override {
        auto denom = dot( normal, r.direction() );

        //no hit if ray is parallel (enough) with plane
        if ( std::fabs( denom ) < epsilon )
            return false;

        //return false if t is outside ray range
        auto t = -dot( normal, r.origin()) / denom;
        if ( !ray_t.contains( t ) ) {
            return false;
        }
        auto isect_p = r.at( t );
        auto alpha = dot( w, cross( isect_p, v ) );
        auto beta = dot( w, cross( u, isect_p ) );

        if (alpha < 0.0f || alpha > 1.0f || beta < 0.0f || beta > 1.0f) {
            return false;
        }

        return true;
    }

    aabb bounding_box() const override {
        // Axis-aligned bbox of the parallelogram spanned by {0, u, v, u+v}
        point3 p0(0,0,0);
        point3 p1 = u;
        point3 p2 = v;
        point3 p3 = u + v;

        point3 minp(
            std::fmin(std::fmin(p0.x(), p1.x()), std::fmin(p2.x(), p3.x())),
            std::fmin(std::fmin(p0.y(), p1.y()), std::fmin(p2.y(), p3.y())),
            std::fmin(std::fmin(p0.z(), p1.z()), std::fmin(p2.z(), p3.z()))
        );
        point3 maxp(
            std::fmax(std::fmax(p0.x(), p1.x()), std::fmax(p2.x(), p3.x())),
            std::fmax(std::fmax(p0.y(), p1.y()), std::fmax(p2.y(), p3.y())),
            std::fmax(std::fmax(p0.z(), p1.z()), std::fmax(p2.z(), p3.z()))
        );
        return aabb(minp, maxp);
    }

    float surface_area() const override {
        return area;
    }

    point3 sample_over_surface() const override {
        double r1 = random_double();
        double r2 = random_double();
        return r1 * u + r2 * v;
    }

    double pdf_w_value(const point3 &p, const vec3 &direction) const override {
        //PDF value is 0 if it doesn't hit this object
        intersection rec;
        ray r = ray( p, direction );
        interval r_interval = interval( epsilon, infinity );
        if ( !intersect( r, r_interval, rec ) ) {
            return 0;
        }

        //This is just a mapping from a uniform distribution over the surface of the object to the projected sphere
        //dw = (dAcos(theta)) / (distance_squared)
        //so (dA/dw) = distance_squared / cos(theta)
        //if the surface area is uniform, then pdfA = 1 / area
        //we have pdfW = pdfA * (dA / dw) = (1 / area) * (distance_squared / cos(theta))
        //    = distance_squared / (cos(theta) * area)

        auto t = rec.get_t();
        auto distance_squared = t * t * direction.length_squared();
        //cos of our solid angle
        auto cosine =  dot( direction, rec.m_normal ) / direction.length();
        if (cosine <= 1e-6) return 0.0;
        double abs_cos = std::fabs( cosine );
        return distance_squared / ( abs_cos * surface_area() );
    }

    double pdf_A_value(const point3 &p) const override {
        return 1 / area;
    }

    vec3 get_normal(const point3& p) const override {
        return normal;
    }


private:
    vec3 u, v;
    vec3 normal;
    float area;

    //Assisting value
    vec3 w;
};

#endif //BENDERER_QUAD_H