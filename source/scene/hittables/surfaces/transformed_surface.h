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
    }


    void compute_properties() override {
        if (m_surface){
            m_surface->compute_properties();
            set_count(m_surface->get_count());
            set_surface_area(m_surface->get_surface_area());
            set_flux_rgb(m_surface->get_flux_rgb());
        }
    }

    void set_explicit_light(bool is_light) override {
        m_surface->set_explicit_light(is_light);
    }

    bool surface_hit( const ray& r, interval ray_t, surface_hit_rec& rec ) const override {
        ray offset_r = m_transform->transform_ray(r);

        //Probability defers to sub-item here
        if ( !m_surface->surface_hit( offset_r, ray_t, rec ) ) {
            return false;
        }


        point3 new_p = m_transform->reverse_transform_point( rec.get_p() );
        vec3 new_n = m_transform->reverse_transform_normal( rec.get_normal() );
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
        child_sample.m_normal = m_transform->reverse_transform_normal(child_sample.m_normal);
        return child_sample;
    }

    aabb bounding_box() const override {
        return bbox;
    }

private:
    shared_ptr<surface> m_surface;
    shared_ptr<transform> m_transform;
    aabb bbox;
};

#endif //BENDERER_TRANSFORMED_H