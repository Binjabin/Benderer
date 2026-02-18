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

    virtual color emitted( const ray& r_in, const surface_hit_rec& rec, double u, double v, const point3& p ) const {
        return m_radiance;
    }

    virtual bool scatter( const ray& r_in, const surface_hit_rec& rec, surface_scatter_rec& srec ) const {
        return false;
    }

    color get_radiance() const { return m_radiance; }

    //The bsdf function. Determines how much light travels from r_in to r_out
    virtual color get_bsdf( const surface_hit_rec& rec ) const {
        return color(1, 1, 1);
    };

protected:
    //TODO: Allow for non-uniform emission...
    color m_radiance = color( 0, 0, 0 );
};

#endif //MATERIAL_H
