//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_PRIMITIVE_H
#define BENDERER_PRIMITIVE_H
#include "../hittable.h"

class primitive : public hittable {

protected:
    primitive(shared_ptr<material> mat) : m_mat(mat) {
        set_count(1);
    }

    shared_ptr<material> m_mat;

    virtual double calculate_surface_area() const = 0;

    void compute_properties() override {
        if (m_mat) {
            double area = calculate_surface_area();
            set_surface_area(area);
            set_flux_rgb(pi * area * m_mat->get_radiance());
        }
    }

    point3 sample_point_over_flux(double seed) const override {
        return sample_over_surface();
    }

    virtual point3 sample_over_surface() const = 0;
};

#endif //BENDERER_PRIMITIVE_H