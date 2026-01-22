//
// Created by binjabin on 1/18/26.
//

#ifndef BENDERER_MEDIUM_HIT_RECORD_H
#define BENDERER_MEDIUM_HIT_RECORD_H
#include <memory>
#include "interaction.h"

//tell compiler we handle what this is later
class material;

class medium_hit {
public:
    medium_hit(const interaction& interaction)
        : m_interaction(interaction){
    }

    ~medium_hit() = default;

    double m_t(){ return m_interaction.t; };

    double m_p(){ return m_interaction.p; }

    double m_time(){ return m_interaction.time; }

    interaction m_interaction;

    //phase function
    //std::shared_ptr<material> m_mat;

    //???????
    //double pdf_v = 0.0;
};
#endif //BENDERER_MEDIUM_HIT_RECORD_H