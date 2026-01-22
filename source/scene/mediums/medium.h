//
// Created by binjabin on 1/18/26.
//

#ifndef BENDERER_MEDIUM_H
#define BENDERER_MEDIUM_H

#include "../../structures/ray.h"
#include "../../structures/intersections/medium_hit_record.h"
#include "../../utility/color.h"

class medium {
public:
    virtual ~medium() = default;

    //Light emitted at point x
    virtual color emitted(const point3& x) const = 0;

    //Absorbtion extinction co-efficient at x
    virtual color sigma_a(const point3& x) const = 0;

    //Scattering extinction co-efficient at x
    virtual color sigma_s(const point3& x) const = 0;

    //For convenience
    virtual color sigma_t(const point3& x) const {
        return sigma_a(x) + sigma_s(x);
    }

    //Upper bound of sigma_t over a ray
    virtual double sigma_t_maj(const ray& r, double t0, double t1) const {

    }

    //Sample a distance, with no interaction (and then at the end an interaction, or our of the medium)
    virtual bool sample(const ray& r, double t0, double tMax, medium_hit_record& rec) const = 0;

    //Bounds within which it can effect rays
    virtual aabb bounds() const = 0;


};

#endif //BENDERER_MEDIUM_H