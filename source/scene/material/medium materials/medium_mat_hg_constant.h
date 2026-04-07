//
// Henyey-Greenstein phase function medium material
//

#ifndef BENDERER_MEDIUM_MAT_HG_CONSTANT_H
#define BENDERER_MEDIUM_MAT_HG_CONSTANT_H

#include "../medium_material.h"
#include "../../../structures/onb.h"

class medium_mat_hg_constant : public medium_material {
public:
    // g: asymmetry parameter. g>0 = forward scattering, g=0 = isotropic, g<0 = backward
    medium_mat_hg_constant(color sigma_a, color sigma_s, color emission, double g)
        : m_sigma_a(sigma_a), m_sigma_s(sigma_s), m_emission(emission), m_g(g) {
        m_sigma_t = sigma_a + sigma_s;
        m_albedo = color(
            m_sigma_t[0] > epsilon ? sigma_s[0] / m_sigma_t[0] : 0,
            m_sigma_t[1] > epsilon ? sigma_s[1] / m_sigma_t[1] : 0,
            m_sigma_t[2] > epsilon ? sigma_s[2] / m_sigma_t[2] : 0
        );
    }

    color sigma_a(const point3& p) const override { return m_sigma_a; }
    color sigma_s(const point3& p) const override { return m_sigma_s; }
    color sigma_t(const point3& p) const override { return m_sigma_t; }
    color albedo(const point3& p) const override { return m_albedo; }
    color emission(const point3& p) const override { return m_emission; }

    medium_properties sample(const point3& p) const override {
        return {m_sigma_t, m_sigma_s, m_emission};
    }

    color sigma_maj() const override { return m_sigma_t; }

    color average_radiance() const override { return m_emission; }

    // Non-IS: sample uniform sphere, evaluate HG for phase_pdf
    void scatter(const vec3& in_dir, medium_scatter_rec& srec) const override {
        sphere_pdf d_pdf = sphere_pdf();
        pdf_rec prec;
        d_pdf.sample(prec);

        srec.s_dir = prec.direction;
        srec.w_pdf = prec.pdf; // 1/(4pi)
        srec.phase_pdf = hg_eval(in_dir, prec.direction);
    }

    // IS: importance-sample HG distribution
    void scatter_is(const vec3& in_dir, medium_scatter_rec& srec) const override {
        // in_dir = -r.direction(), points backward toward camera
        // Forward direction (direction of light travel) = -in_dir
        vec3 forward = unit_vector(-in_dir);

        // Sample cos(theta) from HG distribution
        double cos_theta = sample_hg_cos_theta();
        double sin_theta = std::sqrt(std::max(0.0, 1.0 - cos_theta * cos_theta));
        double phi = 2.0 * pi * random_double();

        // Build local frame around forward direction (z = forward)
        onb uvw(forward);
        vec3 local_dir(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);
        vec3 world_dir = uvw.transform(local_dir);

        double pf = hg_value(cos_theta);
        srec.s_dir = world_dir;
        srec.w_pdf = pf;      // sampling PDF = HG value (perfect IS)
        srec.phase_pdf = pf;   // phase function value
    }

    // Evaluate phase function for given in/out directions
    // in = -r.direction() (backward), out = scattered direction (forward)
    // cos(scattering angle) = dot(-in, out) = dot(forward_propagation, out)
    double phase(const interaction& isect, const vec3& in, const vec3& out) const override {
        return hg_eval(in, out);
    }

private:
    // HG phase function: p(cos_theta) = (1-g^2) / (4*pi*(1+g^2-2*g*cos_theta)^(3/2))
    double hg_value(double cos_theta) const {
        double g2 = m_g * m_g;
        double denom = 1.0 + g2 - 2.0 * m_g * cos_theta;
        return (1.0 - g2) / (4.0 * pi * denom * std::sqrt(denom));
    }

    // Evaluate HG given in/out direction vectors (using code convention)
    double hg_eval(const vec3& in, const vec3& out) const {
        // in points backward, out points toward scatter target
        // Scattering angle cosine = dot(propagation_dir, out) = dot(-in, out)
        double cos_theta = dot(unit_vector(-in), unit_vector(out));
        return hg_value(cos_theta);
    }

    // Importance sample cos(theta) from HG distribution
    double sample_hg_cos_theta() const {
        if (std::abs(m_g) < 1e-3) {
            // Nearly isotropic: sample uniform
            return 1.0 - 2.0 * random_double();
        }
        double xi = random_double();
        double sqr_term = (1.0 - m_g * m_g) / (1.0 - m_g + 2.0 * m_g * xi);
        return (1.0 + m_g * m_g - sqr_term * sqr_term) / (2.0 * m_g);
    }

    color m_albedo;
    color m_sigma_a = uninit_vec;
    color m_sigma_s = uninit_vec;
    color m_sigma_t = uninit_vec;
    color m_emission = uninit_vec;
    double m_g; // asymmetry parameter
};

#endif //BENDERER_MEDIUM_MAT_HG_CONSTANT_H
