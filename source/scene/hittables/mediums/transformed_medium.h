//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_TRANSFORMED_MEDIUM_H
#define BENDERER_TRANSFORMED_MEDIUM_H

#include "medium.h"
#include "../../transforms/transform.h"

class transformed_medium : public medium {
public:
    transformed_medium(shared_ptr<medium> medium, shared_ptr<transform> transform)
        : m_medium(medium), m_transform(transform) {
        bbox = m_transform->transform_bbox(m_medium->bounding_box());
    }

    aabb bounding_box() const override {
        return bbox;
    }

    bool medium_hit(const ray& r, const interval& ray_t, medium_intersections& rec) const override {
        ray offset_r = m_transform->transform_ray(r);

        //Probability defers to sub-item here
        if ( !m_medium->medium_hit( offset_r, ray_t, rec ) ) {
            return false;
        }

        return true;
    }

    void compute_properties() override {
        //Nothing to do!
    }

private:
    shared_ptr<medium> m_medium;
    shared_ptr<transform> m_transform;
    aabb bbox;
};

#endif //BENDERER_TRANSFORMED_MEDIUM_H