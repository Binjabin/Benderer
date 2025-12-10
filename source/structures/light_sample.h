//
// Created by binjabin on 12/7/25.
//

#ifndef BENDERER_LIGHT_SAMPLE_H
#define BENDERER_LIGHT_SAMPLE_H

#include "../benderer.h"

class light_sample {
public:
    virtual ~light_sample() = default;

    light_sample(color r)
        : m_radiance(r) {}

    color m_radiance;
    //pdf value over solid angle steradians
    virtual bool is_environment_light() const = 0;

    virtual vec3 light_direction(point3 o) const = 0;

    virtual double light_distance(point3 o) const = 0;

    virtual double pdf_w_value(point3 o) const = 0;

    virtual double pdf_A_value() const {
        return 0.0;
    };

    virtual vec3 get_normal() const {
        return vec3(0, 0, 0);
    }
};

class environment_light_sample : public light_sample {
public:
    environment_light_sample(const color& r, double w_v, const vec3& d)
        : light_sample(r), m_pdf_w_value(w_v), m_direction(d) {}

    bool is_environment_light() const override {
        return true;
    }

    vec3 light_direction(point3 o) const override {
        return m_direction;
    };

    double pdf_w_value(point3 o) const override {
        return m_pdf_w_value;
    }

    double light_distance(point3 o) const override {
        return infinity;
    }
private:
    vec3 m_direction;
    double m_pdf_w_value;
};

class surface_light_sample : public light_sample {
public:
    surface_light_sample(const color& radiance, const point3& p, const vec3 n, const double a_v)
        : light_sample(radiance), m_light_p(p), m_normal(n), m_pdf_a_value(a_v) {}

    bool is_environment_light() const override {
        return false;
    }

    vec3 light_direction(point3 o) const override {
        return unit_vector(m_light_p - o);
    }

    double light_distance(point3 o) const override {
        return (o - m_light_p).length();
    }

    double pdf_w_value(point3 o) const override {
        auto d = light_distance(o);
        auto scalar = d * d / std::abs(dot(m_normal, -light_direction(o)));
        auto pdf_w = m_pdf_a_value * scalar;
        return pdf_w;
    }

    vec3 get_normal() const override {
        return m_normal;
    }

    double pdf_A_value() const override {
        return m_pdf_a_value;
    }

    double m_pdf_a_value;
    point3 m_light_p;
    vec3 m_normal;
};



#endif //BENDERER_LIGHT_SAMPLE_H