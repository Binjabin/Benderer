//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_MEDIUM_MATERIAL_H
#define BENDERER_MEDIUM_MATERIAL_H

#include "../../utility/color.h"

class medium_material {
public:
    virtual ~medium_material() = default;

    virtual color emitted(const point3& x) const = 0;

    virtual color sigma_a(const point3& x) const = 0;

    virtual color sigma_s(const point3& x) const = 0;

    virtual color sigma_t(const point3& x) const = 0;

    virtual double sigma_t_maj(const ray &r, const interval& ray_t) const = 0;

    virtual bool sample(const ray& r, const interval& ray_t, medium_hit_rec& rec) const = 0;
};

#endif //BENDERER_MEDIUM_MATERIAL_H