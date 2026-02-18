//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_ISOTROPIC_H
#define MAT_ISOTROPIC_H

#include "../source/scene/material/surface_material.h"
#include "../source/scene/texture/texture.h"
#include "../source/scene/texture/textures/tex_solid_colour.h"
#include "../source/structures/pdf.h"

class isotropic : public surface_material {
public:
    isotropic( const color& albedo ) : tex( make_shared<solid_color>( albedo ) ) {
    }

    isotropic( shared_ptr<texture> tex ) : tex( tex ) {
    }

    bool scatter( const ray& r_in, const surface_hit_rec& rec, surface_scatter_rec& srec ) const override {

        srec.attenuation = tex->value( rec.m_intersection );

        sphere_pdf scatter_pdf = sphere_pdf();
        vec3 scattered_dir = scatter_pdf.sample();
        srec.s_ray = ray(rec.get_p() + scattered_dir * epsilon, scattered_dir);

        srec.w_pdf =  scatter_pdf.value(scattered_dir);

        return true;
    }

    double scattering_pdf( const ray& r_in, const surface_hit_rec& rec, const ray& scattered ) const override {
        return 1.0 / ( 4.0 * pi );
    }

    color get_attenuation(const surface_hit_rec &rec) const override {
        return tex->value( rec.u, rec.v, rec.p );
    }

    color bsdf(vec3 d_in, const surface_hit_rec &rec, const vec3 &r_out) override {
        // We don't use importance sampling for this; uniform over sphere
        return get_attenuation(rec) * (1.0 / (4.0 * pi));
    }

private:
    shared_ptr<texture> tex;
};

#endif //MAT_ISOTROPIC_H
