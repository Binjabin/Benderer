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
        if (m_medium){
            m_medium->compute_properties();
            set_count(m_medium->get_count());
            set_volume(m_medium->get_volume());
            set_flux_rgb(m_medium->get_flux());
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

    std::vector<shared_ptr<medium>> flatten() const override {
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
    aabb bbox;

    double m_global_furthest_point;
    double m_local_furthest_point;
    point3 m_origin;
};

#endif //BENDERER_TRANSFORMED_MEDIUM_H