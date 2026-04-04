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

        m_origin = m_transform->transform_point( m_medium->origin());
        m_local_furthest_point = m_medium->local_furthest_point();

        m_global_furthest_point = m_origin.length() + m_local_furthest_point;
    }

    aabb bounding_box() const override {
        return bbox;
    }

    double global_furthest_point() const override {
        return m_global_furthest_point;
    }

    double local_furthest_point() const override {
        return m_local_furthest_point;
    }

    bool medium_hit(const ray& r, const interval& ray_t, medium_intersections& rec) const override {
        //We keep intersection values as t values so don't need to worry about these for transformations
        ray offset_r = m_transform->transform_ray(r);
        if ( !m_medium->medium_hit( offset_r, ray_t, rec ) ) {
            return false;
        }

        return true;
    }

    point3 origin() const override {
        return m_origin;
    }

    void compute_properties() override {
        //Nothing to do!
    }



private:
    shared_ptr<medium> m_medium;
    shared_ptr<transform> m_transform;
    aabb bbox;

    double m_global_furthest_point;
    double m_local_furthest_point;
    point3 m_origin;
};

#endif //BENDERER_TRANSFORMED_MEDIUM_H