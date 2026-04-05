//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_TRANSLATE_H
#define BENDERER_TRANSLATE_H
#include <memory>

#include "transform.h"
#include "../../structures/ray.h"

class translate : public transform {
public:
    translate(const vec3 &offset)
        : offset(offset) {
    }

    ray transform_ray(const ray &r) const override {
        return ray( r.origin() - offset, r.direction(), r.time() );
    };

    point3 reverse_transform_point(const point3 &p) const override {
        return (p + offset);
    }

    point3 transform_point(const point3 &p) const override {
        return (p - offset);
    }

    vec3 reverse_transform_direction(const vec3 &direction) const override {
        return direction;
    }

    vec3 transform_direction(const vec3 &direction) const override {
        return direction;
    }

    aabb transform_bbox(const aabb &bbox) override {
        return bbox + offset;
    }

private:
    vec3 offset;
};

#endif //BENDERER_TRANSLATE_H