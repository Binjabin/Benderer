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
        set_bbox(m_transform->transform_bbox(m_medium->bounding_box()));
        set_origin(m_transform->transform_point(m_medium->origin()));
        set_local_furthest_point(m_medium->local_furthest_point());
        set_global_furthest_point(origin().length() + local_furthest_point());
        set_count(m_medium->get_count());
    }

    bool medium_hit(const ray& r, const interval& ray_t, medium_intersections& rec) const override {
        //We keep intersection values as t values so don't need to worry about these for transformations
        ray offset_r = m_transform->transform_ray(r);

        medium_intersections local_rec;
        if ( !m_medium->medium_hit( offset_r, ray_t, local_rec ) ) return false;

        for (auto& slice : local_rec.mod_slices()) {
            m_transform->reverse_transform_interval(r, slice.m_interval);
        }

        rec.fuse(local_rec);
        return true;
    }

    void compute_properties() override {
        if (m_medium){
            m_medium->compute_properties();
            set_count(m_medium->get_count());
            set_volume(m_medium->get_volume());
            set_flux(m_medium->get_flux());
        }
    }

    void set_explicit_light(bool is_light) override {
        m_medium->set_explicit_light(is_light);
    }

    volume_light_sample sample_light_over_flux(double seed, double running_prob) const override {
        volume_light_sample child_sample = m_medium->sample_light_over_flux(seed, running_prob);
        child_sample.m_light_p = m_transform->reverse_transform_point(child_sample.m_light_p);
        return child_sample;
    }

    double pdf_value(const point3 &origin, const vec3 &direction) const override {
        point3 p = m_transform->transform_point(origin);
        vec3 d = m_transform->transform_direction(direction);
        return m_medium->pdf_value(p, d);
    }

    std::vector<shared_ptr<medium>> flatten() override {
        std::vector<shared_ptr<medium>> flattened;
        auto child_flattened = m_medium->flatten();
        for (auto& child : child_flattened) {
            flattened.push_back(
                make_shared<transformed_medium>(child, m_transform)
            );
        }
        return flattened;
    }

private:
    shared_ptr<medium> m_medium;
    shared_ptr<transform> m_transform;
};

#endif //BENDERER_TRANSFORMED_MEDIUM_H