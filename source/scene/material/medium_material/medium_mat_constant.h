//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_MEDIUM_MAT_CONSTANT_H
#define BENDERER_MEDIUM_MAT_CONSTANT_H
#include "../medium_material.h"
#include "../../../structures/interval.h"
#include "../../../records/medium_hit_rec.h"

class medium_mat_constant : public medium_material {
public:
    medium_mat_constant(const color& c)
        : m_sigma_t(c), m_sigma_a(c * 0.5), m_sigma_s(c * 0.5), m_maj(max_component(c)) {
    }

    color emitted(const point3& x) const override {
        return vec3(0, 0, 0);
    }

    color sigma_a(const point3& x) const override {
        return m_sigma_a;
    }

    color sigma_s(const point3& x) const override {
        return m_sigma_s;
    }

    color sigma_t(const point3& x) const override {
        return m_sigma_t;
    }

    double sigma_t_maj(const ray& r, const interval& ray_t) const override {
        return m_maj;
    }

    bool sample(const ray& r, const interval& ray_t, medium_hit_rec& rec) const override {
        if (m_maj <= 0.0) return false;

        double u = random_double();
        double dist = -std::log(1.0 - u) / m_maj;

        //Leaves volume, no intersect
        double t = ray_t.min + dist;
        if (t >= ray_t.max) return false;


        return true;
    }

private:
    color m_sigma_t;
    color m_sigma_a;
    color m_sigma_s;
    double m_maj;
};

#endif //BENDERER_MEDIUM_MAT_CONSTANT_H