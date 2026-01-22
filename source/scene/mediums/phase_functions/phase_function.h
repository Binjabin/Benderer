//
// Created by binjabin on 1/21/26.
//

#ifndef BENDERER_PHASE_FUNCTION_H
#define BENDERER_PHASE_FUNCTION_H

class phase_function {
    virtual ~phase_function() = default;
    virtual vec3 sample(const vec3& d_in) const = 0;
    virtual double pdf(const vec3& d_in, const vec3& d_out) const = 0;
    virtual double eval(const vec3& d_in, const vec3& d_out) const = 0;
}

#endif //BENDERER_PHASE_FUNCTION_H