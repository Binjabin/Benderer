//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_PRIMITIVE_H
#define BENDERER_PRIMITIVE_H
#include "../hittable.h"
#include "../../../structures/ray.h"

class primitive : public hittable {

protected:
    primitive(shared_ptr<material> mat) : m_mat(mat) {
        set_count(1);
    }

    shared_ptr<material> m_mat;

    virtual double calculate_surface_area() const = 0;

    virtual vec3 get_normal(point3 p) const = 0;

    void compute_properties() override {
        if (m_mat) {
            double area = calculate_surface_area();
            set_surface_area(area);
            set_flux_rgb(pi * area * m_mat->get_radiance());
        }
    }

    void set_explicit_light(bool is_light) {
        m_is_explicit_light = is_light;
    }

    bool hit( const ray& r, interval ray_t, hit_record& rec ) const override {
        bool h = prim_hit(r, ray_t, rec);
        rec.mat = m_mat;
        rec.pdf_v = local_pdf(rec.p);
        rec.is_explicit_light = m_is_explicit_light;
        rec.time = r.time();
        return h;
    }

    virtual bool prim_hit( const ray& r, interval ray_t, hit_record& rec ) const = 0;

    surface_light_sample sample_light_over_flux(double seed, double running_prob) const override {

        vec3 p = sample_over_surface();
        surface_light_sample result;
        result.m_radiance = m_mat->get_radiance();
        result.m_light_p = p;
        result.m_normal = get_normal(p);
        result.m_pdf_A = local_pdf(p) * running_prob;

        return result;
    }

    virtual point3 sample_over_surface() const = 0;

    double local_pdf(point3& p) const {
        return 1.0 / get_surface_area();
    };



private:
   bool m_is_explicit_light = false;

};

#endif //BENDERER_PRIMITIVE_H