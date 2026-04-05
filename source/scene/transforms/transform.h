//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_TRANSFORM_H
#define BENDERER_TRANSFORM_H

#include "../../structures/ray.h"
#include "../../structures/aabb.h"

class transform {
public:
    virtual ray transform_ray(const ray& r) const = 0;

    virtual point3 reverse_transform_point(const point3& p) const = 0;

    virtual point3 transform_point(const point3& p) const = 0;

    virtual vec3 reverse_transform_direction(const vec3& n) const = 0;

    virtual aabb transform_bbox(const aabb& bbox) = 0;

    virtual vec3 transform_direction(const vec3& direction) const = 0;

    virtual ~transform() = default;
};

#endif //BENDERER_TRANSFORM_H