//
// Created by binjabin on 12/7/25.
//

#ifndef BENDERER_LIGHT_SAMPLE_H
#define BENDERER_LIGHT_SAMPLE_H
#include "../benderer.h"

class light_sample {
public:
    light_sample() : objects() {

    }
private:
    ray m_ray;
    color m_color;
    //pdf per unit area. functions as delta
    double m_pdf_area;
};

#endif //BENDERER_LIGHT_SAMPLE_H