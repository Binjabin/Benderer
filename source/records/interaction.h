//
// Created by binjabin on 1/21/26.
//

#ifndef BENDERER_INTERACTION_H
#define BENDERER_INTERACTION_H


class interaction {
public:
    interaction() {
    }
    //How far along ray
    double m_t;
    //Intersection point
    point3 m_p;
    //Time (for motion blur)
    double m_time;
};


#endif //BENDERER_INTERACTION_H