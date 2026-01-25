//
// Created by binjabin on 6/29/25.
//

#ifndef MATERIAL_H
#define MATERIAL_H

#include "../hittables/hittable.h"
#include "../../structures/vec3.h"
#include "../../records/scatter_record.h"

class material {
public:
    //TODO: Allow for non-uniform emission...
    virtual ~material() = default;

    virtual color emitted( const ray& r_in, const surface_hit_rec& rec, double u, double v, const point3& p ) const {
        return m_radiance;
    }

    virtual bool scatter( const ray& r_in, const surface_hit_rec& rec, scatter_record& srec ) const {
        return false;
    }

    virtual double scattering_pdf( const ray& r_in, const surface_hit_rec& rec, const ray& scattered ) const {
        return 0;
    }

    color get_radiance() const { return m_radiance; }

    virtual color get_attenuation( const surface_hit_rec& rec ) const {
        return color(1, 1, 1);
    };

    //The bsdf function. Determines how much light travels from r_in to r_out
    virtual color bsdf(vec3 d_in, const surface_hit_rec& rec, const vec3& r_out) {
        return color(1, 1, 1);
    }

protected:
    //TODO: Allow for non-uniform emission...
    color m_radiance = color( 0, 0, 0 );
};

#endif //MATERIAL_H
