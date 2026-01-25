//
// Created by binjabin on 1/18/26.
//

#ifndef BENDERER_HIT_RECORD_H
#define BENDERER_HIT_RECORD_H
#include "intersection.h"

//tell compiler we handle what this is later
class material;

//The record of a collision with an object
class surface_hit_rec {
public:
    //Probability density of surface in the scene
    double m_pdf_v;
    //The material of the object
    shared_ptr<material> m_mat;
    //whether the object is a light
    bool m_is_explicit_light;

    double get_t() const { return m_intersection.m_interaction.m_t; }
    point3 get_p() const { return m_intersection.m_interaction.m_p; }
    double get_time() const { return m_intersection.m_interaction.m_time; }

    vec3 get_normal() const { return m_intersection.m_normal; }
    double get_u() const { return m_intersection.m_u; }
    double get_v() const { return m_intersection.m_v; }
    bool get_front_face() const { return m_intersection.m_front_face; }

    void transform_to(const point3& pos, const vec3& normal) {
        m_intersection.m_interaction.m_p = pos;
        m_intersection.m_normal = unit_vector(normal);
    }

    intersection m_intersection;
};

#endif //BENDERER_HIT_RECORD_H