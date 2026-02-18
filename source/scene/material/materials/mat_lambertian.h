//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_LAMBERTIAN_H
#define MAT_LAMBERTIAN_H

#include "../../texture/texture.h"
#include "../../texture/textures/tex_solid_colour.h"

class lambertian : public surface_material {
public:
    lambertian( const color& albedo ) : tex( make_shared<solid_color>( albedo ) ) {
    }

    lambertian( const shared_ptr<texture> tex ) : tex( tex ) {
    }

    bool scatter( const ray& r_in, const surface_hit_rec& rec, surface_scatter_rec& srec ) const override {

        cosine_pdf scatter_pdf = cosine_pdf( rec.get_normal() );
        pdf_rec prec;
        scatter_pdf.sample(prec);
        srec.s_ray = ray(rec.get_p() + prec.direction * epsilon, prec.direction);

        srec.bsdf = get_bsdf(rec);
        srec.w_pdf =  prec.pdf;

        return true;
    }

    color get_bsdf(const surface_hit_rec &rec) const override {
        return (1 / pi) * tex->value( rec.m_intersection );
    }

private:
    shared_ptr<texture> tex;
};

#endif //MAT_LAMBERTIAN_H
