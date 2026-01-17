//
// Created by binjabin on 12/9/25.
//

#ifndef BENDERER_SOLID_COLOR_SKYBOX_H
#define BENDERER_SOLID_COLOR_SKYBOX_H
#include "skybox.h"

class solid_color_skybox : public skybox {
public:
    solid_color_skybox(const color& color) :
        m_color(color) {
    }

    color sample_color(vec3 direction) const override {
        return m_color;
    }

    color get_flux_rgb() const override {
        return 4 * pi * m_color;
    }

    environment_light_sample sample_light_over_flux(double running_p) const override {
        auto d = random_unit_vector();
        //TODO: Make neater
        auto pdf_w = running_p / (4.0 * pi);

        environment_light_sample result;
        result.m_radiance = m_color;
        result.m_direction = d;
        result.m_pdf_w = pdf_w;

        return result;
    }

    double get_pdf_value(vec3 d) const override {
        return 1 / (4 * pi);
    }

private:
    const color m_color;
};

#endif //BENDERER_SOLID_COLOR_SKYBOX_H