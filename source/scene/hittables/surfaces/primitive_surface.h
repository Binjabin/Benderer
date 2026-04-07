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
        set_bbox(m_shape->bounding_box());
        set_origin(point3(0, 0, 0));
        set_local_furthest_point(m_shape->furthest_point());
        set_global_furthest_point(m_shape->furthest_point());
    }

    void compute_properties() override {
        if (m_mat) {
            double area = m_shape->surface_area();
            set_surface_area(area);
            set_flux(pi * area * m_mat->average_radiance());
        }
    }

    void set_explicit_light(bool is_light) override {
        m_is_explicit_light = is_light;
    }

    bool surface_hit( const ray& r, const interval& r_t, surface_hit_rec& rec ) const override {
        if (bounding_box().hit(r, r_t) == false) return false;

        intersection isect;
        bool h = m_shape->intersect(r, r_t, isect);
        if(!h) { return false; }

        rec.m_intersection = isect;
        rec.m_mat = m_mat;
        rec.m_pdf_v = m_shape->pdf_A_value(rec.get_p());
        rec.m_is_explicit_light = m_is_explicit_light;

        return h;
    }

    double pdf_value(const point3 &origin, const vec3 &direction) const override {
        surface_hit_rec rec;
        vec3 d = unit_vector(direction);
        ray r(origin, d);
        if (!surface_hit(r, interval(0, infinity), rec)) {
            return 0.0;
        }

        const vec3 to_light = rec.get_p() - origin;
        const double dist2 = to_light.length_squared();
        if (dist2 <= epsilon) return 0.0;

        const double cos_light = dot(rec.get_normal(), -d);
        if (cos_light <= epsilon) return 0.0;

        return rec.m_pdf_v * (dist2 / cos_light);
    }

    bool surface_hit_check(const ray &r, interval r_t) const override {
        if (bounding_box().hit(r, r_t) == false) return false;
        bool h = m_shape->intersect_check(r, r_t);
        return h;
    }

    surface_light_sample sample_light_over_flux(double seed, double running_prob) const override {

        vec3 p = m_shape->sample_over_surface();
        surface_light_sample result;
        result.m_radiance = m_mat->average_radiance();
        result.m_light_p = p;
        result.m_normal = m_shape->get_normal(p);
        result.m_pdf_A = m_shape->pdf_A_value(p) * running_prob;

        return result;
    }

    std::vector<shared_ptr<surface>> flatten() override {
        std::vector<shared_ptr<surface>> flattened;
        flattened.push_back(shared_from_this());
        return flattened;
    }

private:
    bool m_is_explicit_light = false;
    shared_ptr<shape> m_shape;
    shared_ptr<surface_material> m_mat;

};

#endif //BENDERER_PRIMITIVE_H