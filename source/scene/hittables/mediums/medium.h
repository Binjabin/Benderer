//
// Created by binjabin on 1/18/26.
//

#ifndef BENDERER_MEDIUM_H
#define BENDERER_MEDIUM_H

#include "../hittable.h"
#include "../../../records/medium_intersection.h"
#include "../../../structures/ray.h"
#include "../../../utility/Color/color.h"

class medium : public hittable {
public:
    virtual ~medium() = default;

    virtual bool medium_hit(const ray& r, const interval& ray_t, medium_intersections& rec) const = 0;
};

#endif //BENDERER_MEDIUM_H