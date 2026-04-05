//
// Created by binjabin on 1/21/26.
//

#ifndef BENDERER_SPHERE_H
#define BENDERER_SPHERE_H
#include "solid.h"
#include "../../../structures/ray.h"

class sphere : public solid {
public:
    sphere(double rad) : radius(rad), radius2(rad * rad) {
    }

    bool intersect(const ray &r, interval ray_t, intersection& isect) const override {
        vec3 oc = r.origin();
        auto a = r.direction().length_squared();
        auto h = dot( r.direction(), oc );
        auto c = oc.length_squared() - radius * radius;
        auto discriminant = h * h - a * c;

        if ( discriminant < 0 ) {
            return false;
        }

        auto sqrt_d = std::sqrt( discriminant );

        //try both roots of equation
        auto root = ( -h - sqrt_d ) / a;
        if ( !ray_t.surrounds( root ) ) {
            root = ( -h + sqrt_d ) / a;
            if ( !ray_t.surrounds( root ) ) {
                return false;
            }
        }

        auto p = r.at( root );
        isect.set_interaction_values(root, p, r.time());

        vec3 outward_normal = p / radius;
        // A hit is on the front face if the ray is hitting the outside surface
        isect.m_front_face = (dot(r.direction(), outward_normal) < 0);
        // Always orient the geometric normal against the incoming ray
        isect.m_normal = isect.m_front_face ? outward_normal : -outward_normal;

        get_sphere_uv( outward_normal, isect.m_u, isect.m_v );

        return true;
    }

    bool intersect_check(const ray &r, interval ray_t) const override {
        vec3 oc = r.origin();
        auto a = r.direction().length_squared();
        auto h = dot( r.direction(), oc );
        auto c = oc.length_squared() - radius * radius;
        auto discriminant = h * h - a * c;

        if ( discriminant < 0 ) {
            return false;
        }

        auto sqrt_d = std::sqrt( discriminant );

        //try both roots of equation
        auto root = ( -h - sqrt_d ) / a;
        if ( !ray_t.surrounds( root ) ) {
            root = ( -h + sqrt_d ) / a;
            if ( !ray_t.surrounds( root ) ) {
                return false;
            }
        }

        return true;
    }

    bool contains(const point3& p) const override {
        return (p.length_squared() <= radius2);
    }

    aabb bounding_box() const override {
        auto rvec = vec3( radius, radius, radius );
        return aabb( -rvec, rvec );
    }

    float surface_area() const override {
        return 4 * pi * radius2;
    }

    point3 sample_over_surface() const override {
        double u = random_double();
        double v = random_double();

        double z = 1.0 - 2.0 * u;
        double phi = 2.0 * pi * v;
        double r_xy = std::sqrt(std::fmax(0.0, 1.0 - z * z));

        vec3 local = vec3(r_xy * std::cos(phi), r_xy * std::sin(phi), z) * radius;
        return local;
    }

    double pdf_w_value(const point3& o, const vec3& v) const override {

        ray r = ray( o, v );
        if ( !intersect_check( r, interval( epsilon, infinity ) ) ) {
            return 0;
        }

        auto dist_squared = o.length_squared();

        //Insides sphere, so full angle
        if (dist_squared <= radius2) {
            return 1.0 / (4.0 * pi);
        }

        auto cos_theta_max = std::sqrt( 1 - radius * radius / dist_squared );
        //the "patch" of view that the sphere takes up
        auto solid_angle = 2 * pi * ( 1 - cos_theta_max );

        //the PDF for this patch of the sphere
        return 1 / solid_angle;
    }

    double pdf_A_value(const point3& p) const override {
        return 1 / (4 * pi * radius2);
    }

    vec3 get_normal(const point3& p) const override {
        return unit_vector(p);
    }

    double furthest_point() const override {
        return radius;
    }

    double volume() const override {
        return 4.0 / 3.0 * pi * radius * radius * radius;
    }

    point3 sample_over_volume() const override {
        vec3 dir = random_unit_vector();
        double r = radius * std::cbrt(random_double());
        return dir * r;
    }

    double pdf_V_value(const point3& p) const override {
        return 1 / ((4.0 / 3.0) * pi * radius3);
    }

private:
    double radius;

    //for efficiency
    double radius2 = radius * radius;
    double radius3 = radius * radius * radius;

    static void get_sphere_uv( const point3& p, double& u, double& v ) {
        // p: a given point on the sphere of radius one
        // u, v are the returned latitude and longitude

        auto theta = std::acos( -p.y() );
        auto phi = std::atan2( -p.z(), p.x() ) + pi;

        u = phi / ( 2 * pi );
        v = theta / pi;
    }
};

#endif //BENDERER_SPHERE_H