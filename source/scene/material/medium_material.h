//
// Created by binjabin on 1/29/26.
//

#ifndef BENDERER_MEDIUM_MATERIAL_H
#define BENDERER_MEDIUM_MATERIAL_H

#include "../../records/medium_hit_rec.h"
#include "../../records/medium_scatter_rec.h"
#include "../../records/pdf_rec.h"
#include "../hittables/hittable.h"
#include "../../structures/vec3.h"
#include "../../records/surface_scatter_rec.h"

class medium_material {
public:
    virtual ~medium_material() = default;

    //The probability density of absorption
    virtual color sigma_a(const point3& p) const = 0;

    //The probability density of scattering
    virtual color sigma_s(const point3&) const = 0;

    //emittance per unit distance
    virtual color emission(const point3&) const = 0;

    //Total of sigma_a and sigma_s - extinction
    virtual color sigma_t(const point3&) const = 0;

    //Sigma_a/Sigma_t. Chance of scattering (compared to absorption)
    virtual color albedo(const point3&) const = 0;

    //Defines how a ray scatters
    virtual double phase(const interaction& isect, const vec3& in, const vec3& out) const = 0;

    //Generate a scatter direction from Phase function - Importance Sampled
    virtual void scatter(const vec3& in_dir, medium_scatter_rec& srec) const = 0;

    //Generate a scatter direction from Phase function - Importance Sampled
    virtual void scatter_is(const vec3& in_dir, medium_scatter_rec& srec) const = 0;

    virtual color sigma_maj() const = 0;
};

#endif //BENDERER_MEDIUM_MATERIAL_H