//
// Created by binjabin on 12/26/25.
//

#ifndef BENDERER_PATH_STATE_H
#define BENDERER_PATH_STATE_H
#include "../structures/vec3.h"

struct path_state {
    vec3 prev_vertex_position;
    double prev_bsdf_pdf;

    vec3 overall_throughput;
    int depth;

    double rr_probability;

};
#endif //BENDERER_PATH_STATE_H