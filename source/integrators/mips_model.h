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
    shared_ptr<light_sample> sample_over_flux(const hittable &lights, const shared_ptr<skybox> sky, point3& from) const {
        double sky_weight = sky->get_flux_weight();
        double lights_weight = lights.get_flux_weight();
        double flux_sum = sky_weight + lights_weight;

        double sample = flux_sum * random_double();

        if (sample < sky_weight) {
            //Sample from sky
            double p = sky_weight / flux_sum;
            return sky->sample_light_over_flux(p);
        }
        double new_seed = (sample - sky_weight) / lights_weight;
        double p = lights_weight / flux_sum;
        return lights.sample_light_over_flux(new_seed, p);
    }
};

#endif //BENDERER_MIPS_MODEL_H