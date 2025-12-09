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

    bool hit( const ray& r, interval ray_t, hit_record& rec ) const override {
        ray offset_r( r.origin() - offset, r.direction(), r.time() );

        if ( !m_object->hit( offset_r, ray_t, rec ) ) {
            return false;
        }

        rec.p += offset;

        return true;
    }

    aabb bounding_box() const override {
        return bbox;
    }

private:
    vec3 offset;
    aabb bbox;
};

#endif //BENDERER_TRANSLATE_H