//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_INTEGRATOR_H
#define BENDERER_INTEGRATOR_H

#include "../benderer.h"
#include "../scene/hittables/hittable.h"

class integrator {
public:
    virtual color ray_color( const ray& r, int depth, const hittable& world, const hittable& lights, const color& background ) const {
        return color(0,0,0);
    }
};

#endif //BENDERER_INTEGRATOR_H