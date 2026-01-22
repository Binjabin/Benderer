//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_RTW_MODEL_H
#define BENDERER_RTW_MODEL_H
#include "../source/integrators/integrator.h"
#include "../source/structures/pdf.h"
#include "../source/records/scatter_record.h"
#include "../source/scene/material/material.h"

class rtw_model : public integrator {
public:
    color ray_color(const ray &r, int depth, const hittable &world, const hittable &lights, const shared_ptr<skybox> sky) const override {
        //no more light is gathered
        if ( depth <= 0 ) {
            return color( 0, 0, 0 );
        }

        surface_hit rec;
        auto ray_interval = interval( epsilon, infinity );

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

        if ( srec.skip_pdf ) {
            return srec.attenuation * ray_color( srec.skip_pdf_ray, depth - 1, world, lights, sky );
        }

        auto light_ptr = make_shared<hittable_pdf>( lights, rec.p );
        mixture_pdf p( light_ptr, srec.pdf_ptr );

        ray scattered = ray( rec.p, p.generate(), r.time() );
        auto pdf_value = p.value( scattered.direction() );

        double scattering_pdf = rec.mat->scattering_pdf( r, rec, scattered );

        color sample_color = ray_color( scattered, depth - 1, world, lights, sky );
        color color_from_scatter = ( srec.attenuation * scattering_pdf * sample_color ) / pdf_value;

        return color_from_emission + color_from_scatter;
    }
};

#endif //BENDERER_RTW_MODEL_H