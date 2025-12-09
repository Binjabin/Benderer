//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_RTW_MODEL_H
#define BENDERER_RTW_MODEL_H
#include "integrator.h"
#include "../structures/pdf.h"
#include "../structures/scatter_record.h"
#include "../scene/material/material.h"

class rtw_model : public integrator {
public:
    color ray_color(const ray &r, int depth, const hittable &world, const hittable &lights, const color& background) const override {
        //no more light is gathered
        if ( depth <= 0 ) {
            return color( 0, 0, 0 );
        }

        hit_record rec;
        auto ray_interval = interval( 0.001, infinity );

        if ( !world.hit( r, ray_interval, rec ) ) {
            return background;
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

        if ( srec.skip_pdf ) {
            return srec.attenuation * ray_color( srec.skip_pdf_ray, depth - 1, world, lights, background );
        }

        auto light_ptr = make_shared<hittable_pdf>( lights, rec.p );
        mixture_pdf p( light_ptr, srec.pdf_ptr );

        ray scattered = ray( rec.p, p.generate(), r.time() );
        auto pdf_value = p.value( scattered.direction() );

        double scattering_pdf = rec.mat->scattering_pdf( r, rec, scattered );

        color sample_color = ray_color( scattered, depth - 1, world, lights, background );
        color color_from_scatter = ( srec.attenuation * scattering_pdf * sample_color ) / pdf_value;

        return color_from_emission + color_from_scatter;

        //SKYBOX
        //vec3 unit_direction = unit_vector( r.direction() );
        //auto a = 0.5 * ( unit_direction.y() + 1.0 );
        //return ( 1.0 - a ) * color( 1.0, 1.0, 1.0 ) + a * color( 0.5, 0.5, 0.7 );
    }
};

#endif //BENDERER_RTW_MODEL_H