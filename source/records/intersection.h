//
// Created by binjabin on 1/21/26.
//

#ifndef BENDERER_INTERSECTION_H
#define BENDERER_INTERSECTION_H
#include "interaction.h"

//The record of an intersection between a surface and a ray
class intersection {
public:
    intersection() : m_interaction(interaction()) {
    }

    void set_interaction_values(double t, const point3& p, double time) {
        m_interaction.m_t = t;
        m_interaction.m_p = p;
        m_interaction.m_time = time;
    }

    //For texture co-ordinates
    double m_u = uninit;
    double m_v = uninit;
    //normal at surface
    vec3 m_normal = uninit_vec;
    //Is this intersection "landing" on the outside face
    bool m_front_face = false;
    interaction m_interaction;

    double get_t() const { return m_interaction.m_t; }
    point3 get_p() const { return m_interaction.m_p; }
    double get_time() const { return m_interaction.m_time; }
};

#endif //BENDERER_INTERSECTION_H