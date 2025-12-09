//
// Created by binjabin on 12/9/25.
//

#ifndef BENDERER_SKYBOX_H
#define BENDERER_SKYBOX_H
#include "../../utility/color.h"

class skybox {
public:
    virtual ~skybox() = default;

    virtual color sample_color(vec3 direction) const = 0;

    virtual color get_flux_rgb() const = 0;

    virtual double get_flux_weight() const {
        return flux_weight(get_flux_rgb());
    };

    virtual vec3 sample_direction_over_flux() const = 0;
};

#endif //BENDERER_SKYBOX_H