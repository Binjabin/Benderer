//
// Created by binjabin on 12/9/25.
//

#ifndef BENDERER_MIPS_MODEL_H
#define BENDERER_MIPS_MODEL_H
#include "integrator.h"
#include "../structures/scatter_record.h"
#include "../scene/material/material.h"

class mips_model : public integrator {
public:
    color ray_color(const ray &r, int depth, const hittable &world, const hittable &lights, const shared_ptr<skybox> sky) const override {
        //no more light is gathered
        if ( depth <= 0 ) {
            return color( 0, 0, 0 );
        }

        hit_record rec;
        auto ray_interval = interval( 0.001, infinity );

        if ( !world.hit( r, ray_interval, rec ) ) {
            return sky->sample_color(r.direction());
        }


        scatter_record srec;
        color color_from_emission = rec.mat->emitted( r, rec, rec.u, rec.v, rec.p );

        //cast ray, get results
        //After this, pdf_value is the pdf of the actually generated ray through our SAMPLING SYSTEM.
        //ie how we chose the ray
        //we use this to weight the contribution later
        if ( !rec.mat->scatter( r, rec, srec ) ) {
            return color_from_emission;
        }

        return srec.attenuation * ray_color( srec.skip_pdf_ray, depth - 1, world, lights, sky );
    }

private:
    vec3 sample_over_flux(const hittable &lights, const shared_ptr<skybox> sky, point3& from) const {
        double sky_weight = sky->get_flux_weight();
        double flux_sum = sky_weight;
        flux_sum += lights.get_flux_weight();

        double sample = flux_sum * random_double();

        if (sample < sky_weight) {
            //Sample from sky
            double new_seed = sample / flux_sum;
            return sky->sample_direction_over_flux();
        }
        else {
            double new_seed = (sample - sky_weight) / flux_sum;
            point3 light_point = lights.sample_point_over_flux(new_seed);
            return unit_vector(light_point - from);
        }
    }
};

#endif //BENDERER_MIPS_MODEL_H