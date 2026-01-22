//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_ISOTROPIC_H
#define MAT_ISOTROPIC_H

#include "../material.h"
#include "../../texture/texture.h"
#include "../../texture/textures/tex_solid_colour.h"
#include "../../../structures/pdf.h"

class isotropic : public material {
public:
    isotropic( const color& albedo ) : tex( make_shared<solid_color>( albedo ) ) {
    }

    isotropic( shared_ptr<texture> tex ) : tex( tex ) {
    }

    bool scatter( const ray& r_in, const surface_hit& rec, scatter_record& srec ) const override {
        srec.attenuation = tex->value( rec.m_intersection );
        srec.pdf_ptr = make_shared<sphere_pdf>();
        srec.skip_pdf = false;
        return true;
    }

    double scattering_pdf( const ray& r_in, const surface_hit& rec, const ray& scattered ) const override {
        return 1.0 / ( 4.0 * pi );
    }

    color get_attenuation(const surface_hit &rec) const override {
        return tex->value( rec.u, rec.v, rec.p );
    }

    color bsdf(vec3 d_in, const surface_hit &rec, const vec3 &r_out) override {
        // We don't use importance sampling for this; uniform over sphere
        return get_attenuation(rec) * (1.0 / (4.0 * pi));
    }

private:
    shared_ptr<texture> tex;
};

#endif //MAT_ISOTROPIC_H
