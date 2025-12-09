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
        double area = calculate_surface_area();
        set_surface_area(area);
        set_flux_rgb(pi * area * m_mat->get_radiance());
    }

    virtual double calculate_surface_area() {
        return 0;
    }

    shared_ptr<material> m_mat;

};

#endif //BENDERER_PRIMITIVE_H