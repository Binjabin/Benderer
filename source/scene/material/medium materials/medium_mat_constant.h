//
// Created by binjabin on 1/30/26.
//

#ifndef BENDERER_MEDIUM_MAT_CONSTANT_H
#define BENDERER_MEDIUM_MAT_CONSTANT_H
#include "../medium_material.h"

class medium_mat_constant : public medium_material {
public:
    medium_mat_constant(color sigma_a, color sigma_s, color emission)
        : m_sigma_a(sigma_a), m_sigma_s(sigma_s), m_emission(emission) {
        m_sigma_t = sigma_a + sigma_s;
        m_albedo = sigma_s / m_sigma_t;
    }

    medium_mat_constant(color albedo, double density, color emission)
        : medium_mat_constant(albedo, color(density, density, density), emission) {}

    color sigma_a(const point3& p) const override {
        return m_sigma_a;
    }

    color sigma_s(const point3& p) const override {
        return m_sigma_s;
    }

    color sigma_t(const point3& p) const override {
        return m_sigma_t;
    }

    color albedo(const point3& p) const override {
        return m_albedo;
    }

    color emission(const point3& p) const override {
        return m_emission;
    }

    color sigma_maj() const override {
        return m_sigma_t;
    }

    void scatter(const vec3 &in_dir, medium_scatter_rec &srec) const override {
        sphere_pdf d_pdf = sphere_pdf();
        pdf_rec prec;
        d_pdf.sample(prec);

        srec.w_pdf = prec.pdf;
        srec.s_dir = prec.direction;
        srec.phase_pdf = 1.0 / (4.0 * pi);
    }

    void scatter_is(const vec3& in_dir, medium_scatter_rec &srec) const override {
        sphere_pdf d_pdf = sphere_pdf();
        pdf_rec prec;
        d_pdf.sample(prec);

        srec.w_pdf = prec.pdf;
        srec.s_dir = prec.direction;
        srec.phase_pdf = 1.0 / (4.0 * pi);
    }

    double phase(const interaction &isect, const vec3 &in, const vec3 &out) const override {
        return 1.0 / (4.0 * pi);
    }

private:
    color m_albedo;
    float m_density;

    //Absorption Co-Efficient
    color m_sigma_a = uninit_vec;
    //Scattering Co-Efficient
    color m_sigma_s = uninit_vec;
    color m_sigma_t = uninit_vec;

    color m_emission = uninit_vec;
    sphere_pdf pdf;
};

#endif //BENDERER_MEDIUM_MAT_CONSTANT_H