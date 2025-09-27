//
// Created by binjabin on 7/8/25.
//

#ifndef QUAD_H
#define QUAD_H

#include "../hittable.h"

class quad : public hittable {
public:
    quad( const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat )
        : Q( Q ), u( u ), v( v ), mat( mat ) {
        auto n = cross( u, v );
        normal = unit_vector( n );
        D = dot( normal, Q );
        w = n / dot( n, n );

        //get area from cross product length
        area = n.length();

        set_bounding_box();
    }

    virtual void set_bounding_box() {
        auto bbox_diagonal1 = aabb( Q, Q + u + v );
        auto bbox_diagonal2 = aabb( Q + u, Q + v );
        bbox = aabb( bbox_diagonal1, bbox_diagonal2 );
    }

    aabb bounding_box() const override { return bbox; }

    bool hit( const ray& r, interval ray_t, hit_record& rec ) const override {
        auto denom = dot( normal, r.direction() );

        //no hit if ray is parallel (enough) with plane
        if ( std::fabs( denom ) < 1e-8 )
            return false;

        //return false if t is outside ray range
        auto t = ( D - dot( normal, r.origin() ) ) / denom;
        if ( !ray_t.contains( t ) )
            return false;

        auto intersection = r.at( t );
        vec3 planar_hitpt_vector = intersection - Q;
        auto alpha = dot( w, cross( planar_hitpt_vector, v ) );
        auto beta = dot( w, cross( u, planar_hitpt_vector ) );

        if ( !is_interior( alpha, beta, rec ) ) {
            return false;
        }

        rec.t = t;
        rec.p = intersection;
        rec.mat = mat;
        rec.set_face_normal( r, normal );

        return true;
    }

    virtual bool is_interior( double a, double b, hit_record& rec ) const {
        interval unit_interval = interval( 0, 1 );
        if ( !unit_interval.contains( a ) || !unit_interval.contains( b ) ) return false;
        rec.u = a;
        rec.v = b;
        return true;
    }

    double pdf_value( const point3& origin, const vec3& direction ) const override {
        //pdf value is 0 if it doesn't hit this object
        hit_record rec;
        ray r = ray( origin, direction );
        interval r_interval = interval( 0.001, infinity );
        if ( !this->hit( r, r_interval, rec ) ) {
            return 0;
        }

        //This is just a mapping from a uniform distribution over the surface of the object to the prjected sphere

        //dw = (dAcos(theta)) / (distance_squared)
        //so (dA/dw) = distance_squared / cos(theta)

        //if the surface area is uniform then pdfA = 1 / area

        //we have pdfW = pdfA * (dA / dw) = (1 / area) * (distance_squared / cos(theta))
        //    = distance_squared / (cos(theta) * area)


        auto distance_squared = rec.t * rec.t * direction.length_squared();
        //cos of our solid angle
        auto cosine = std::fabs( dot( direction, rec.normal ) / direction.length() );
        return distance_squared / ( cosine * area );
    }

    vec3 random( const point3& origin ) const override {
        //gets a random place on the surface
        auto p = Q + ( random_double() * u ) + ( random_double() * v );
        return p - origin;
    }

private:
    point3 Q;
    vec3 u, v;
    vec3 w;
    shared_ptr<material> mat;
    aabb bbox;

    vec3 normal;
    double D;
    double area;
};

inline shared_ptr<hittable_list> box( const point3& a, const point3& b, shared_ptr<material> mat ) {
    auto sides = make_shared<hittable_list>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    auto min = point3( std::fmin( a.x(), b.x() ), std::fmin( a.y(), b.y() ), std::fmin( a.z(), b.z() ) );
    auto max = point3( std::fmax( a.x(), b.x() ), std::fmax( a.y(), b.y() ), std::fmax( a.z(), b.z() ) );

    auto dx = vec3( max.x() - min.x(), 0, 0 );
    auto dy = vec3( 0, max.y() - min.y(), 0 );
    auto dz = vec3( 0, 0, max.z() - min.z() );

    sides->add( make_shared<quad>( point3( min.x(), min.y(), max.z() ), dx, dy, mat ) ); // front
    sides->add( make_shared<quad>( point3( max.x(), min.y(), max.z() ), -dz, dy, mat ) ); // right
    sides->add( make_shared<quad>( point3( max.x(), min.y(), min.z() ), -dx, dy, mat ) ); // back
    sides->add( make_shared<quad>( point3( min.x(), min.y(), min.z() ), dz, dy, mat ) ); // left
    sides->add( make_shared<quad>( point3( min.x(), max.y(), max.z() ), dx, -dz, mat ) ); // top
    sides->add( make_shared<quad>( point3( min.x(), min.y(), min.z() ), dx, dz, mat ) ); // bottom

    return sides;
}


#endif //QUAD_H
