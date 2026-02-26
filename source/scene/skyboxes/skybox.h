//
// Created by binjabin on 12/9/25.
//

#ifndef BENDERER_SKYBOX_H
#define BENDERER_SKYBOX_H
#include "../../utility/color/color.h"

class skybox {
public:
    virtual ~skybox() = default;

    virtual color sample_color(vec3 direction) const = 0;

    virtual color get_flux_rgb() const = 0;

    virtual double get_flux_weight() const {
        return luminance(get_flux_rgb());
    };

    virtual environment_light_sample sample_light_over_flux(double running_p) const = 0;

    virtual double get_pdf_value(vec3 d) const = 0;
};

#endif //BENDERER_SKYBOX_H