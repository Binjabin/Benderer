//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_LAMBERTIAN_H
#define MAT_LAMBERTIAN_H

#include "../../texture/texture.h"
#include "../../texture/textures/tex_solid_colour.h"

class lambertian : public material {
public:
    lambertian( const color& albedo ) : tex( make_shared<solid_color>( albedo ) ) {
    }

    lambertian( const shared_ptr<texture> tex ) : tex( tex ) {
    }

    bool scatter( const ray& r_in, const hit_record& rec, scatter_record& srec ) const override {
        srec.attenuation = tex->value( rec.u, rec.v, rec.p );
        srec.pdf_ptr = make_shared<cosine_pdf>( rec.normal );
        srec.skip_pdf = false;
        return true;
    }

    double scattering_pdf( const ray& r_in, const hit_record& rec, const ray& scattered ) const override {
        auto cos_theta = dot( rec.normal, unit_vector( scattered.direction() ) );
        return cos_theta < 0 ? 0 : cos_theta / pi;
    }

    color get_attenuation(const hit_record &rec) const override {
        return tex->value( rec.u, rec.v, rec.p );
    }

private:
    shared_ptr<texture> tex;
};

#endif //MAT_LAMBERTIAN_H
