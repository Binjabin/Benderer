//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_PRIMITIVE_H
#define BENDERER_PRIMITIVE_H
#include "surface.h"
#include "../hittable.h"
#include "../../../structures/ray.h"
#include "../../shapes/shape.h"

class primitive_surface : public surface {

public:
    primitive_surface(shared_ptr<shape> shape, shared_ptr<surface_material> mat)
        : m_shape(shape),  m_mat(mat) {
        set_count(1);
        bbox = m_shape->bounding_box();
    }

    void compute_properties() override {
        if (m_mat) {
            double area = m_shape->surface_area();
            set_surface_area(area);
            set_flux_rgb(pi * area * m_mat->get_radiance());
        }
    }

    void set_explicit_light(bool is_light) override {
        m_is_explicit_light = is_light;
    }

    bool surface_hit( const ray& r, interval ray_t, surface_hit_rec& rec ) const override {
        intersection isect;
        bool h = m_shape->intersect(r, ray_t, isect);
        if(!h) { return false; }

        rec.m_intersection = isect;
        rec.m_mat = m_mat;
        rec.m_pdf_v = m_shape->pdf_A_value(rec.get_p());
        rec.m_is_explicit_light = m_is_explicit_light;

        return h;
    }


    surface_light_sample sample_light_over_flux(double seed, double running_prob) const override {

        vec3 p = m_shape->sample_over_surface();
        surface_light_sample result;
        result.m_radiance = m_mat->get_radiance();
        result.m_light_p = p;
        result.m_normal = m_shape->get_normal(p);
        result.m_pdf_A = m_shape->pdf_A_value(p) * running_prob;

        return result;
    }

    aabb bounding_box() const override { return bbox; }

private:
    bool m_is_explicit_light = false;
    shared_ptr<shape> m_shape;
    shared_ptr<surface_material> m_mat;
    aabb bbox;


};

#endif //BENDERER_PRIMITIVE_H