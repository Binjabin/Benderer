//
// Created by binjabin on 12/9/25.
//

#ifndef BENDERER_GRADIENT_SKYBOX_H
#define BENDERER_GRADIENT_SKYBOX_H
#include "skybox.h"

class gradient_skybox : public skybox {
public:
    gradient_skybox(const color& top, const color& bottom) : m_top(top), m_bottom(bottom) {}

    color sample_color(vec3 direction) const override {
        vec3 unit_direction = unit_vector( direction );
        auto a = 0.5 * ( unit_direction.y() + 1.0 );
        return ( 1.0 - a ) * m_top + a * m_bottom;
    }

    color get_flux_rgb() const override {
        return 4 * pi * ( m_top + m_bottom ) * 0.5;
    }

    //TODO: improve?
    shared_ptr<environment_light_sample> sample_light_over_flux(double running_p) const override {
        auto d = random_unit_vector();
        auto c = sample_color(d);
        //TODO: Make neater
        auto pdf_omega = running_p / (4.0 * pi);
        environment_light_sample sample = environment_light_sample(c, pdf_omega, d);
        return make_shared<environment_light_sample>(sample);
    }

private:
    const color m_top;
    const color m_bottom;
};

#endif //BENDERER_GRADIENT_SKYBOX_H