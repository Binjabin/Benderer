//
// Created by binjabin on 12/7/25.
//

#ifndef BENDERER_LIGHT_SAMPLE_H
#define BENDERER_LIGHT_SAMPLE_H

#include "../benderer.h"


//A sample given a position
struct local_light_sample {
    color m_radiance;
    vec3 m_direction;
    double m_pdf_w;
    double m_distance;
    //Accounts for the facing direction and distance of physically based lights. Should just be 1 for environment lights.
    double m_geometry_term;

    bool m_is_env_light;
};

struct environment_light_sample {
    color m_radiance;
    vec3 m_direction;
    double m_pdf_w;

    local_light_sample to_local_sample(point3 from) const {
        local_light_sample res;
        res.m_radiance = m_radiance;
        res.m_direction = unit_vector(m_direction);
        res.m_pdf_w = m_pdf_w;
        res.m_distance = infinity;
        res.m_geometry_term = 1;
        res.m_is_env_light = true;
        return res;
    }
};

struct light_ray_sample {
    color m_radiance;
    double m_pdf_w;
    ray m_ray;
    bool m_is_env_light;
};

struct surface_light_sample {
    color m_radiance;
    point3 m_light_p;
    vec3 m_normal;
    double m_pdf_A;

    //Direction from o to the light
    vec3 direction(point3 o) const {
        return unit_vector(m_light_p - o);
    }

    double distance(point3 o) const {
        return (o - m_light_p).length();
    }

    double squared_distance(point3 o) const {
        return (o - m_light_p).length_squared();
    }

    double pdf_w(const point3& from) const {
        double dist2 = squared_distance(from);
        if (dist2 <= epsilon) return 0.0;
        double cos_theta = dot(m_normal, -direction(from));
        if (cos_theta <= epsilon) return 0;
        return m_pdf_A * dist2 / cos_theta;
    }

    double geometry_term(const point3& from) const {
        double dist2 = squared_distance(from);
        if (dist2 <= epsilon) return 0.0;
        double cos_theta = cos_term(-direction(from));
        if (cos_theta <= epsilon) return 0;
        return (cos_theta / dist2);
    }

    double cos_term(const vec3& dir) const {
        return dot(m_normal, dir);
    }

    local_light_sample to_local_sample(point3 from) const {
        local_light_sample res;
        res.m_radiance = m_radiance;
        res.m_direction = direction(from);
        res.m_pdf_w = pdf_w(from);
        res.m_distance = distance(from);
        res.m_geometry_term = geometry_term(from);
        res.m_is_env_light = false;
        return res;
    }

    
};

struct volume_light_sample {
    color m_radiance;
    point3 m_light_p;
    double m_pdf_V;

    //Direction from o to the light
    vec3 direction(point3 o) const {
        return unit_vector(m_light_p - o);
    }

    double distance(point3 o) const {
        return (o - m_light_p).length();
    }

    double squared_distance(point3 o) const {
        return (o - m_light_p).length_squared();
    }

    double pdf_w(const point3& from) const {
        double dist2 = squared_distance(from);
        if (dist2 <= epsilon) return 0.0;
        return m_pdf_V * dist2;
    }

    local_light_sample to_local_sample(point3 from) const {
        local_light_sample res;
        res.m_radiance = m_radiance;
        res.m_direction = direction(from);
        res.m_pdf_w = pdf_w(from);
        res.m_distance = distance(from);
        res.m_geometry_term = 1.0;
        res.m_is_env_light = false;
        return res;
    }


};


#endif //BENDERER_LIGHT_SAMPLE_H