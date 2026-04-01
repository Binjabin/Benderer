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

    color bsdf(const intersection& i, const vec3& in, const vec3& out) override {
        if (dot(out, i.m_normal) <= 0.0) return color(0, 0, 0);
        return tex->value(i.m_u, i.m_v, i.get_p()) * inv_pi;
    }

    double pdf(const intersection &i, const vec3 &in, const vec3 &out) override {
        double cos_theta = dot(i.m_normal, out);
        cos_theta = std::max(cos_theta, 0.0);
        return cos_theta * inv_pi;
    }

    bool scatter_is(const intersection& i, const vec3& in, surface_scatter_rec& srec) override {
        cosine_pdf scatter_pdf = cosine_pdf( i.m_normal );
        pdf_rec prec;
        scatter_pdf.sample(prec);

        srec.s_ray = ray(i.get_p() + prec.direction * epsilon, prec.direction, i.get_time());
        srec.w_pdf =  prec.pdf;
        srec.bsdf = bsdf(i, in, srec.s_ray.direction());
        srec.is_delta = false;

        return true;
    }

    bool scatter(const intersection& i, const vec3& in, surface_scatter_rec& srec) override {
        hemisphere_pdf scatter_pdf = hemisphere_pdf( i.m_normal );
        pdf_rec prec;
        scatter_pdf.sample(prec);

        srec.s_ray = ray(i.get_p() + prec.direction * epsilon, prec.direction, i.get_time());
        srec.bsdf = bsdf(i, in, srec.s_ray.direction());
        srec.is_delta = false;

        return true;
    }
    

    color emission(const intersection& i) const override {
        return color(0, 0, 0);
    }

    color get_radiance() const override {
        return colors::black;
    }

    bool is_delta() const override {
        return false;
    }

private:
    shared_ptr<texture> tex;
};

#endif //MAT_LAMBERTIAN_H
