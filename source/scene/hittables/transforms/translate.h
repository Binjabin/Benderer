//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_TRANSLATE_H
#define BENDERER_TRANSLATE_H
#include "transform.h"

class translate : public transform {
public:
    translate( shared_ptr<hittable> object, const vec3& offset )
        : transform(object), offset( offset ) {
        bbox = object->bounding_box() + offset;
    }

    aabb bounding_box() const override {
        return bbox;
    }

    ray transform_ray(const ray &r) const override {
        return ray( r.origin() - offset, r.direction(), r.time() );
    };

    point3 reverse_transform_point(const point3 &p) const override {
        return (p + offset);
    }

    point3 reverse_transform_normal(const vec3 &n) const override {
        return n;
    }

private:
    vec3 offset;
    aabb bbox;
};

#endif //BENDERER_TRANSLATE_H