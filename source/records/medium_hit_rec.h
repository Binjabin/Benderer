//
// Created by binjabin on 1/30/26.
//

#ifndef BENDERER_MEDIUM_HIT_REC_H
#define BENDERER_MEDIUM_HIT_REC_H
#include "interaction.h"
class medium_material;

class medium_hit_rec {
public:
    medium_hit_rec()
        : m_interaction(interaction()), m_transmittance(colors::white), m_is_scatter(false), m_mat(nullptr) {
    }

    ~medium_hit_rec() = default;

    double m_t() const { return m_interaction.m_t; }

    void set_t(double t) { m_interaction.m_t = t; }

    point3 m_p() const { return m_interaction.m_p; }

    void set_p(point3 p) { m_interaction.m_p = p; }

    double m_time() const { return m_interaction.m_time; }

    interaction m_interaction;

    //How much color gets through the medium (Up to the sampled event or t.max if no event)
    color m_transmittance;

    //Is this event a scatter event (or instead an absorption event)
    bool m_is_scatter;

    double m_mat_pdf;

    color m_emission;

    //The material we scattered off
    shared_ptr<medium_material> m_mat = nullptr;
};

#endif //BENDERER_MEDIUM_HIT_REC_H