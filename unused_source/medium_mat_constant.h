//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_MEDIUM_MAT_CONSTANT_H
#define BENDERER_MEDIUM_MAT_CONSTANT_H
#include "medium_material.h"
#include "../source/structures/interval.h"
#include "../source/records/medium_intersection.h"

class medium_mat_constant : public medium_material {
public:
    medium_mat_constant(const color& c, double sigma_t)
        : m_albedo(c), m_sigma_t(sigma_t) {
        m_sigma_a = (color(1.0, 1.0, 1.0) - c) * sigma_t;
        m_sigma_s = c * sigma_t;
    }

    color emitted(const point3& x) const override {
        return vec3(0, 0, 0);
    }

    bool sample(const ray& r, const interval& ray_t, interaction& i) const override {
        if (m_sigma_t <= 0.0) return false;

        double u = random_double();
        double dist = -std::log(1.0 - u) / m_sigma_t;

        //Leaves volume, no intersect
        double t = ray_t.min + r.distance_to_t(dist);
        if (t >= ray_t.max) return false;

        i.m_t = t;
        i.m_p = r.at(t);
        i.m_time = r.time();

        return true;
    }

    bool scatter(const ray &r, const medium_intersection &rec, medium_scatter_rec &srec) const override {
        //TODO: Physically accurate here is more complex and requires spectral stuff... Revisit later!
        /*
        double avg = (m_albedo.x() + m_albedo.y() + m_albedo.z()) / 3.0;
        double scatter_prob = std::clamp(avg, 0.0, 1.0);
        double u = random_double();
        if (u < scatter_prob) {
            sphere_pdf pdf = sphere_pdf();
            vec3 d = pdf.generate();
            ray s_ray = ray(rec.m_p() + d * epsilon, d, rec.m_time());

            srec.albedo = m_albedo;
            srec.s_ray = s_ray;
            srec.w_pdf = pdf.value(d);
            srec.phase_pdf = 1 / (4 * pi);

            return true;
        }
        */

        sphere_pdf pdf = sphere_pdf();
        vec3 d = pdf.sample();
        ray s_ray = ray(rec.m_p() + d * epsilon, d, rec.m_time());

        srec.albedo = m_albedo;
        srec.s_ray = s_ray;
        srec.w_pdf = pdf.value(d);
        srec.phase_pdf = 1 / (4 * pi);

        return true;

        return false;
    }



private:
    //Total extinction
    color m_sigma_t;
    //Probability density of absorption (per unit)
    double m_sigma_a;
    //Probability density of scattering (per unit)
    //scattering can be out-scattering, or in scattering
    color m_sigma_s;
    //Emission (per unit) - Light is added
    color m_Le;
    color m_albedo;
};

#endif //BENDERER_MEDIUM_MAT_CONSTANT_H