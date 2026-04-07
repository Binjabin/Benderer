//
// Created by Junie on 4/6/26.
//

#ifndef BENDERER_BOX_H
#define BENDERER_BOX_H

#include "solid.h"
#include "../../../structures/ray.h"

class box : public solid {
public:
    box(const vec3& half_size) : m_half_size(half_size) {
        m_bbox = aabb(-m_half_size, m_half_size);
    }

    bool intersect(const ray &r, interval ray_t, intersection& isect) const override {
        double t_min = ray_t.min;
        double t_max = ray_t.max;

        for (int i = 0; i < 3; ++i) {
            double inv_d = 1.0 / r.direction()[i];
            double t0 = (-m_half_size[i] - r.origin()[i]) * inv_d;
            double t1 = (m_half_size[i] - r.origin()[i]) * inv_d;
            if (inv_d < 0.0) std::swap(t0, t1);

            t_min = std::max(t_min, t0);
            t_max = std::min(t_max, t1);

            if (t_max < t_min) return false;
        }

        double root = t_min;

        if (root <= ray_t.min || root >= ray_t.max) {
            root = t_max;
            if (root <= ray_t.min || root >= ray_t.max) return false;
        }

        point3 p = r.at(root);
        isect.set_interaction_values(root, p, r.time());

        // Normal calculation
        double n_x = 0.0, n_y = 0.0, n_z = 0.0;
        double eps = 1e-5;
        if (std::abs(p.x() - m_half_size.x()) < eps) n_x = 1;
        else if (std::abs(p.x() + m_half_size.x()) < eps) n_x = -1;
        else if (std::abs(p.y() - m_half_size.y()) < eps) n_y = 1;
        else if (std::abs(p.y() + m_half_size.y()) < eps) n_y = -1;
        else if (std::abs(p.z() - m_half_size.z()) < eps) n_z = 1;
        else if (std::abs(p.z() + m_half_size.z()) < eps) n_z = -1;
        vec3 normal(n_x, n_y, n_z);

        isect.m_front_face = (dot(r.direction(), normal) < 0);
        isect.m_normal = isect.m_front_face ? normal : -normal;
        
        return true;
    }

    bool intersect_check(const ray &r, interval ray_t) const override {
        double t_min = -infinity;
        double t_max = +infinity;

        for (int i = 0; i < 3; ++i) {
            double inv_d = 1.0 / r.direction()[i];
            double t0 = (-m_half_size[i] - r.origin()[i]) * inv_d;
            double t1 = (m_half_size[i] - r.origin()[i]) * inv_d;
            if (inv_d < 0.0) std::swap(t0, t1);
            t_min = std::max(t_min, t0);
            t_max = std::min(t_max, t1);
            if (t_max <= t_min) return false;
        }

        if (!ray_t.surrounds(t_min) && !ray_t.surrounds(t_max)) return false;

        return true;
    }

    bool contains(const point3& p) const override {
        return (std::abs(p.x()) <= m_half_size.x() + epsilon &&
                std::abs(p.y()) <= m_half_size.y() + epsilon &&
                std::abs(p.z()) <= m_half_size.z() + epsilon);
    }

    aabb bounding_box() const override {
        return m_bbox;
    }

    float surface_area() const override {
        vec3 d = m_half_size * 2.0;
        return 2.0 * (d.x()*d.y() + d.x()*d.z() + d.y()*d.z());
    }

    point3 sample_over_surface() const override {
        // Not used for volume lights in this way
        return point3(0,0,0);
    }

    double pdf_w_value(const point3& o, const vec3& v) const override {
        return 0; 
    }

    double pdf_A_value(const point3& p) const override {
        return 1.0 / surface_area();
    }

    vec3 get_normal(const point3& p) const override {
        vec3 normal(0,1,0);
        double eps = 1e-5;
        if (std::abs(p.x() - m_half_size.x()) < eps) normal = vec3(1,0,0);
        else if (std::abs(p.x() + m_half_size.x()) < eps) normal = vec3(-1,0,0);
        else if (std::abs(p.y() - m_half_size.y()) < eps) normal = vec3(0,1,0);
        else if (std::abs(p.y() + m_half_size.y()) < eps) normal = vec3(0,-1,0);
        else if (std::abs(p.z() - m_half_size.z()) < eps) normal = vec3(0,0,1);
        else if (std::abs(p.z() + m_half_size.z()) < eps) normal = vec3(0,0,-1);
        return normal;
    }

    double furthest_point() const override {
        return m_half_size.length();
    }

    double volume() const override {
        vec3 d = m_half_size * 2.0;
        return d.x() * d.y() * d.z();
    }

    point3 sample_over_volume() const override {
        return point3(random_double(-m_half_size.x(), m_half_size.x()),
                      random_double(-m_half_size.y(), m_half_size.y()),
                      random_double(-m_half_size.z(), m_half_size.z()));
    }

    double pdf_V_value(const point3& p) const override {
        return 1.0 / volume();
    }

private:
    vec3 m_half_size;
    aabb m_bbox;
};

#endif //BENDERER_BOX_H
