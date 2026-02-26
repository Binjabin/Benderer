//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_MEDIUM_MATERIAL_H
#define BENDERER_MEDIUM_MATERIAL_H

#include "../source/utility/color/color.h"
#include "../source/records/medium_intersection.h"
#include "../source/records/medium_scatter_rec.h"

class medium_material {
public:
    virtual ~medium_material() = default;

    virtual color emitted(const point3& x) const = 0;

    //Get free flight distance (as ray param)!
    virtual bool sample(const ray& r, const interval& ray_t, interaction& i) const = 0;

    virtual bool scatter(const ray& r, const medium_intersection& rec, medium_scatter_rec& srec) const = 0;


};

#endif //BENDERER_MEDIUM_MATERIAL_H