//
// Created by binjabin on 12/7/25.
//


#ifndef BENDERER_SCATTER_RECORD_H
#define BENDERER_SCATTER_RECORD_H

#include "../structures/pdf.h"

class surface_scatter_rec {
public:
    ray s_ray;
    //bsdf color value
    color bsdf = uninit_vec;
    //pdf of direction sampling distribution
    double w_pdf = uninit;
    bool is_spec = false;
};

#endif //BENDERER_SCATTER_RECORD_H