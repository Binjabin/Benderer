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

    shared_ptr<surface_light_sample> sample_light_over_flux(double seed, double running_prob) const override {
        auto p = sample_over_surface();
        auto n = get_normal(p);
        color radiance = m_mat->get_radiance();

        auto pdf_a = local_pdf(p) * running_prob;

        return make_shared<surface_light_sample>(radiance, p, n, pdf_a);
    }

    virtual point3 sample_over_surface() const = 0;

    double local_pdf(point3& p) const {
        return 1.0 / get_surface_area();
    };


};

#endif //BENDERER_PRIMITIVE_H