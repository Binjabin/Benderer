//
// Created by binjabin on 12/26/25.
//

#ifndef BENDERER_PATH_STATE_H
#define BENDERER_PATH_STATE_H
#include "../structures/vec3.h"

struct path_state {

    vec3 overall_throughput;
    int depth;

    double rr_probability;

    //So we can check if we did direct sampling at the last bounce
    bool prev_was_delta = false;
    double prev_bsdf_pdf;
    point3 prev_p;

    static path_state initial_path_state() {
        path_state p_state;
        //Here depth starts from 0 and counts upwards
        p_state.depth = 0;
        p_state.overall_throughput = colors::white;

        p_state.prev_bsdf_pdf = 1.0;
        p_state.prev_was_delta = false;
        p_state.prev_p = point3(0,0,0);

        return p_state;
    }

};
#endif //BENDERER_PATH_STATE_H