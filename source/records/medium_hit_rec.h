//
// Created by binjabin on 1/18/26.
//

#ifndef BENDERER_MEDIUM_HIT_RECORD_H
#define BENDERER_MEDIUM_HIT_RECORD_H
#include <memory>
#include "interaction.h"

//tell compiler we handle what this is later
class material;

class medium_hit_rec {
public:
    medium_hit_rec()
        : m_interaction(interaction()){
    }

    ~medium_hit_rec() = default;

    double m_t() const { return m_interaction.m_t; };

    point3 m_p() const { return m_interaction.m_p; }

    double m_time() const { return m_interaction.m_time; }

    interaction m_interaction;

    //phase function
    //std::shared_ptr<material> m_mat;

    //???????
    //double pdf_v = 0.0;

    void transform_to(const point3& pos) {
        m_interaction.m_p = pos;
    }
};
#endif //BENDERER_MEDIUM_HIT_RECORD_H