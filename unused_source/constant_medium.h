//
// Created by binjabin on 7/9/25.
//

#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "../source/scene/hittables/hittable.h"
#include "../source/scene/material/material.h"
#include "../source/scene/material/materials/mat_isotropic.h"
#include "../source/scene/shapes/solids/solid.h"
#include "../source/scene/texture/texture.h"

class constant_medium : public hittable {
public:
    constant_medium( shared_ptr<solid> boundary, double density, shared_ptr<texture> tex )
        : boundary( boundary ), neg_inv_density( -1 / density ), phase_function( make_shared<isotropic>( tex ) ) {
        set_count(boundary->get_count());
    }

    constant_medium( shared_ptr<hittable> boundary, double density, const color& albedo )
        : boundary( boundary ), neg_inv_density( -1 / density ), phase_function( make_shared<isotropic>( albedo ) ) {
        set_count(boundary->get_count());
    }

    bool hit( const ray& r, interval ray_t, surface_hit& rec ) const override {
        surface_hit rec1, rec2;

        //make sure we actually hit the boundary
        if ( !boundary->hit( r, interval::universe, rec1 ) ) return false;

        //make sure we exit the volume
        if ( !boundary->hit( r, interval( rec1.t + epsilon, infinity ), rec2 ) ) return false;

        //crop enter and exit points to the section of the ray we are checking
        if ( rec1.t < ray_t.min ) rec1.t = ray_t.min;
        if ( rec2.t > ray_t.max ) rec2.t = ray_t.max;

        //if we spend no time in the volume, exit
        if ( rec1.t >= rec2.t ) return false;

        //at earliest, start ray transmission at camera
        if ( rec1.t < 0 ) rec1.t = 0;

        //calculate the distance we spend inside the boundary
        auto ray_length = r.direction().length();
        auto distance_inside_boundary = ( rec2.t - rec1.t ) * ray_length;

        //how long until we deflect in volume
        auto hit_distance = neg_inv_density * std::log( random_double() );

        //if we don't refract, we don't "hit" volume
        if ( hit_distance > distance_inside_boundary ) return false;

        //we hit after hit_distance length from entering the volume
        rec.t = rec1.t + hit_distance / ray_length;
        rec.p = r.at( rec.t );

        rec.normal = vec3( 1, 0, 0 ); //arbitrary normal and front face
        rec.front_face = true;
        rec.mat = phase_function;

        rec.time = r.time();
        rec.is_volume = true;

        return true;
    }

    aabb bounding_box() const override { return boundary->bounding_box(); }

    // Volume is not an emitting surface itself; mirror the boundary properties
    void compute_properties() override {
        boundary->compute_properties();
        set_count(boundary->get_count());
        set_surface_area(boundary->get_surface_area());
        set_flux_rgb(boundary->get_flux_rgb());
    }

    void set_explicit_light(bool is_light) override {
        boundary->set_explicit_light(is_light);
    }

    surface_light_sample sample_light_over_flux(double seed, double running_prob) const override {
        return boundary->sample_light_over_flux(seed, running_prob);
    }

private:
    shared_ptr<hittable> boundary;
    double neg_inv_density;
    shared_ptr<material> phase_function;
};


#endif //CONSTANT_MEDIUM_H
