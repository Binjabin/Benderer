//
// Created by binjabin on 1/21/26.
//

#ifndef BENDERER_INTERACTION_H
#define BENDERER_INTERACTION_H
#include "../benderer.h"

class interaction {
public:
    interaction() {
    }
    //How far along ray
    double m_t = uninit;
    //Intersection point
    point3 m_p = uninit_vec;
    //Time (for motion blur)
    double m_time = uninit;
};


#endif //BENDERER_INTERACTION_H