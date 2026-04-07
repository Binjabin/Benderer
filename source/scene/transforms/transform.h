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

    void reverse_transform_interval(const ray &r, interval &t) const {
        ray r_local = transform_ray(r);

        auto map_t = [&](double t_local) {
            point3 p_local = r_local.at(t_local);
            point3 p_world = reverse_transform_point(p_local);
            const vec3 d = r.direction();
            return dot(p_world - r.origin(), d) / dot(d, d);
        };

        double t0 = map_t(t.min);
        double t1 = map_t(t.max);

        t = interval(std::min(t0, t1), std::max(t0, t1));
    }

    virtual ~transform() = default;
};

#endif //BENDERER_TRANSFORM_H