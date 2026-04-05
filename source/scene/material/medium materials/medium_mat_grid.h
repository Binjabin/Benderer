//
// Created by binjabin on 1/30/26.
//

#ifndef BENDERER_MEDIUM_MAT_CONSTANT_H
#define BENDERER_MEDIUM_MAT_CONSTANT_H
#include <algorithm>

#include "../medium_material.h"
#include "../../../structures/density_grid.h"

class medium_mat_constant : public medium_material {
public:
    medium_mat_constant(shared_ptr<density_grid> grid, shared_ptr<medium_material> base)
        : m_density_grid(grid), m_base(base){
        m_sigma_maj = compute_majorant();
        m_average_density = average_density();
    }


    color sigma_a(const point3& p) const override {
        float d = m_density_grid->sample_density(p);
        return m_base->sigma_a(p) * d;
    }

    color sigma_s(const point3& p) const override {
        float d = m_density_grid->sample_density(p);
        return m_base->sigma_s(p) * d;
    }

    color sigma_t(const point3& p) const override {
        float d = m_density_grid->sample_density(p);
        return m_base->sigma_t(p) * d;
    }

    color albedo(const point3& p) const override {
        return m_base->albedo(p);
    }

    color emission(const point3& p) const override {
        float d = m_density_grid->sample_density(p);
        return m_base->sigma_t(p) * d;
    }

    color sigma_maj() const override {
        return m_sigma_maj;
    }

    void scatter(const vec3 &in_dir, medium_scatter_rec &srec) const override {
        m_base->scatter(in_dir, srec);
    }

    void scatter_is(const vec3& in_dir, medium_scatter_rec &srec) const override {
        m_base->scatter_is(in_dir, srec);
    }

    double phase(const interaction &isect, const vec3 &in, const vec3 &out) const override {
        return 1.0 / (4.0 * pi);
    }

    color average_radiance() const override {
        return m_average_density * m_base->average_radiance();
    }

private:
    shared_ptr<density_grid> m_density_grid;
    shared_ptr<medium_material> m_base;
    color m_sigma_maj;
    double m_average_density;

    color compute_majorant() {
        double m_sigma_maj_r = 0.0;
        double m_sigma_maj_g = 0.0;
        double m_sigma_maj_b = 0.0;

        for (double d : m_density_grid->data) {
            color sigma_t = m_base->sigma_maj();
            m_sigma_maj_r = std::max(sigma_t[0] * d, m_sigma_maj_r);
            m_sigma_maj_g = std::max(sigma_t[1] * d, m_sigma_maj_g);
            m_sigma_maj_b = std::max(sigma_t[2] * d, m_sigma_maj_b);
        }

        return color(m_sigma_maj_r, m_sigma_maj_g, m_sigma_maj_b);
    }

    double average_density() {
        double density_sum = 0.0;
        for (double d : m_density_grid->data) {
            density_sum += d;
        }
        return density_sum / m_density_grid->data.size();
    }

};

#endif //BENDERER_MEDIUM_MAT_CONSTANT_H