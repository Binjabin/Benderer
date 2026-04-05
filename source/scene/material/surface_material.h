//
// Created by binjabin on 6/29/25.
//

#ifndef MATERIAL_H
#define MATERIAL_H

#include "../hittables/hittable.h"
#include "../../structures/vec3.h"
#include "../../records/surface_scatter_rec.h"

class surface_material {
public:
    //TODO: Allow for non-uniform emission...
    virtual ~surface_material() = default;

    virtual color bsdf(const intersection& i, const vec3& in, const vec3& out) = 0;

    virtual double pdf(const intersection& i, const vec3& in, const vec3& out) = 0;

    virtual bool scatter_is(const intersection& i, const vec3& in, surface_scatter_rec& srec) = 0;

    virtual bool scatter(const intersection& i, const vec3& in, surface_scatter_rec& srec) = 0;

    virtual color emission(const intersection& i) const = 0;

    virtual color average_radiance() const = 0;

    virtual bool is_delta() const = 0;

protected:
    //TODO: Allow for non-uniform emission...
    color m_radiance = color( 0, 0, 0 );
};



#endif //MATERIAL_H
