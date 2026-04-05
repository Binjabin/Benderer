//
// Created by binjabin on 1/21/26.
//

#ifndef BENDERER_TRANSFORMED_H
#define BENDERER_TRANSFORMED_H
#include "../hittable.h"
#include "../../transforms/transform.h"

class transformed_surface : public surface {
public:
    transformed_surface(shared_ptr<surface> object, shared_ptr<transform> transform)
        : m_surface(object), m_transform(transform) {
        bbox = m_transform->transform_bbox(m_surface->bounding_box());

        m_origin = m_transform->transform_point( object->origin());
        m_local_furthest_point = object->local_furthest_point();

        m_global_furthest_point = m_origin.length() + m_local_furthest_point;
    }


    void compute_properties() override {
        if (m_surface){
            m_surface->compute_properties();
            set_count(m_surface->get_count());
            set_surface_area(m_surface->get_surface_area());
            set_flux_rgb(m_surface->get_flux());
        }
    }

    void set_explicit_light(bool is_light) override {
        m_surface->set_explicit_light(is_light);
    }

    bool surface_hit( const ray& r, const interval& ray_t, surface_hit_rec& rec ) const override {
        ray offset_r = m_transform->transform_ray(r);

        //Probability defers to sub-item here
        if ( !m_surface->surface_hit( offset_r, ray_t, rec ) ) {
            return false;
        }

        point3 new_p = m_transform->reverse_transform_point( rec.get_p() );
        vec3 new_n = m_transform->reverse_transform_direction( rec.get_normal() );
        rec.transform_to(new_p, new_n);

        return true;
    }

    bool surface_hit_check( const ray& r, interval ray_t ) const override {
        ray offset_r = m_transform->transform_ray(r);
        return m_surface->surface_hit_check( offset_r, ray_t );
    }

    surface_light_sample sample_light_over_flux(double seed, double running_prob) const override {
        surface_light_sample child_sample = m_surface->sample_light_over_flux(seed, running_prob);
        child_sample.m_light_p = m_transform->reverse_transform_point(child_sample.m_light_p);
        child_sample.m_normal = m_transform->reverse_transform_direction(child_sample.m_normal);
        return child_sample;
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

    vec3 origin() const override {
        return m_origin;
    }

    double pdf_value(const point3 &origin, const vec3 &direction) const override {
        point3 p = m_transform->transform_point(origin);
        vec3 local_dir = m_transform->transform_direction(direction);
        return m_surface->pdf_value(p, local_dir);
    }

    std::vector<shared_ptr<surface>> flatten() const override {
        std::vector<shared_ptr<surface>> flattened;

        auto child_flattened = m_surface->flatten();

        for (auto& child : child_flattened) {
            flattened.push_back(
                make_shared<transformed_surface>(child, m_transform)
            );
        }

        return flattened;
    }



private:
    shared_ptr<surface> m_surface;
    shared_ptr<transform> m_transform;
    aabb bbox;

    double m_global_furthest_point;
    double m_local_furthest_point;
    point3 m_origin;
};

#endif //BENDERER_TRANSFORMED_H