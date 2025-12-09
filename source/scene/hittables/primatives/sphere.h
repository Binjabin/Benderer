//
// Created by binjabin on 6/24/25.
//

#ifndef SPHERE_H
#define SPHERE_H

#include "../hittable.h"
#include "../../../structures/onb.h"

class sphere : public primitive {
public:
    sphere( const point3& static_center, double radius, shared_ptr<material> mat ) :
    primitive(mat),
    center( static_center, vec3( 0, 0, 0 ) ),
    radius( std::fmax( 0, radius ) ), mat( mat ) {
        auto rvec = vec3( radius, radius, radius );
        bbox = aabb( static_center - rvec, static_center + rvec );
    }

    sphere( const point3& center1, const point3& center2, double radius, shared_ptr<material> mat ) :
    primitive(mat),
    center( center1, center2 - center1 ),
    radius( std::fmax( 0, radius ) ), mat( mat ) {
        auto rvec = vec3( radius, radius, radius );
        aabb bbox1 = aabb( center1 - rvec, center1 + rvec );
        aabb bbox2 = aabb( center2 - rvec, center2 + rvec );
        bbox = aabb( bbox1, bbox2 );

    }

    bool hit( const ray& r, interval ray_t, hit_record& rec ) const override {
        point3 current_center = center.at( r.time() );
        vec3 oc = current_center - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot( r.direction(), oc );
        auto c = oc.length_squared() - radius * radius;
        auto discriminant = h * h - a * c;

        if ( discriminant < 0 ) {
            return false;
        }

        auto sqrt_d = std::sqrt( discriminant );

        //try both roots of equation
        auto root = ( h - sqrt_d ) / a;
        if ( !ray_t.surrounds( root ) ) {
            root = ( h + sqrt_d ) / a;
            if ( !ray_t.surrounds( root ) ) {
                return false;
            }
        }

        rec.t = root;
        rec.p = r.at( rec.t );
        vec3 outward_normal = ( rec.p - current_center ) / radius;
        rec.set_face_normal( r, outward_normal );
        get_sphere_uv( outward_normal, rec.u, rec.v );
        rec.mat = mat;

        return true;
    }

    aabb bounding_box() const override {
        return bbox;
    }

    double pdf_value( const point3& origin, const vec3& direction ) const override {
        // Only works for stationary spheres

        hit_record rec;
        if ( !this->hit( ray( origin, direction ), interval( 0.001, infinity ), rec ) ) {
            return 0;
        }

        auto dist_squared = ( center.at( 0 ) - origin ).length_squared();
        auto cos_theta_max = std::sqrt( 1 - radius * radius / dist_squared );
        //the "patch" of view that the sphere takes up
        auto solid_angle = 2 * pi * ( 1 - cos_theta_max );

        //the PDF for this patch of the sphere
        return 1 / solid_angle;
    }

    vec3 random( const point3& origin ) const override {
        vec3 direction = center.at( 0 ) - origin;
        auto distance_squared = direction.length_squared();
        onb uvw( direction );
        return uvw.transform( random_to_sphere( radius, distance_squared ) );
    }

    double calculate_surface_area() const override {
        return 4 * pi * radius * radius;
    }

private:
    ray center;
    double radius;
    shared_ptr<material> mat;
    aabb bbox;

    static void get_sphere_uv( const point3& p, double& u, double& v ) {
        // p: a given point on the sphere of radius one
        // u, v are the returned latitude and longitude

        auto theta = std::acos( -p.y() );
        auto phi = std::atan2( -p.z(), p.x() ) + pi;

        u = phi / ( 2 * pi );
        v = theta / pi;
    }

    static vec3 random_to_sphere( double radius, double distance_squared ) {
        auto r1 = random_double();
        auto r2 = random_double();
        auto z = 1 + r2 * ( std::sqrt( 1 - radius * radius / distance_squared ) - 1 );

        auto phi = 2 * pi * r1;
        auto root = std::sqrt( 1 - z * z );
        auto x = std::cos( phi ) * root;
        auto y = std::sin( phi ) * root;

        return vec3( x, y, z );
    }
};


#endif //SPHERE_H
