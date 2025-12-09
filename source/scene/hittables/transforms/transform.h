//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_TRANSFORM_H
#define BENDERER_TRANSFORM_H
#include "../hittable.h"

class transform : public hittable {
public:
    void compute_properties() override {
        if (m_object){
            m_object->compute_properties();
            set_count(m_object->get_count());
            set_surface_area(m_object->get_surface_area());
            set_flux_rgb(m_object->get_flux_rgb());
        }
    }

    bool hit( const ray& r, interval ray_t, hit_record& rec ) const override {
        ray offset_r = transform_ray(r);

        if ( !m_object->hit( offset_r, ray_t, rec ) ) {
            return false;
        }

        rec.p = reverse_transform_point( rec.p );

        rec.normal = reverse_transform_normal( rec.normal );

        return true;
    }

    virtual ray transform_ray(const ray& r) const = 0;

    virtual point3 reverse_transform_point(const point3& p) const = 0;

    virtual vec3 reverse_transform_normal(const vec3& n) const = 0;

    point3 sample_point_over_flux(double seed) const override {
        return reverse_transform_point(m_object->sample_point_over_flux(seed));
    }

protected:
    transform(shared_ptr<hittable> object) : m_object(object) {
    }

    shared_ptr<hittable> m_object;
};

#endif //BENDERER_TRANSFORM_H