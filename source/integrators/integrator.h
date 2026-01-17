//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_INTEGRATOR_H
#define BENDERER_INTEGRATOR_H

#include "../benderer.h"

class scene;

class integrator {
public:
    virtual ~integrator() = default;
    virtual color ray_color( const ray& r, int depth, const world& world ) const {
        return color(0,0,0);
    }
};

#endif //BENDERER_INTEGRATOR_H