//
// Created by binjabin on 1/25/26.
//

#ifndef BENDERER_MEDIUM_SCATTER_RECORD_H
#define BENDERER_MEDIUM_SCATTER_RECORD_H


#include "../structures/pdf.h"

class medium_scatter_rec {
public:
    vec3 s_dir;
    //Like bsdf value for volumes
    double phase_pdf = uninit;
    //pdf value used to select distribution
    double w_pdf = uninit;
};

#endif //BENDERER_MEDIUM_SCATTER_RECORD_H