//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_DIFFUSE_LIGHT_H
#define MAT_DIFFUSE_LIGHT_H

#include "../../texture/texture.h"

class emissive : public surface_material {
public:
    //TODO: Allow for non-uniform emission...
    emissive( shared_ptr<texture> tex ) : tex( tex ) {
        m_emission_color = tex->get_base_color();
    }

    emissive( const color& emit ) : tex( make_shared<solid_color>( emit ) ) {
        m_emission_color = emit;
    }

    color bsdf(const intersection &i, const vec3 &in, const vec3 &out) override {
        return vec3(0, 0, 0);
    }

    double pdf(const intersection &i, const vec3 &in, const vec3 &out) override {
        return uninit;
    }

    bool scatter_is(const intersection& i, const vec3& in, surface_scatter_rec& srec) override {
        return false;
    }

    bool scatter(const intersection& i, const vec3& in, surface_scatter_rec& srec) override {
        return false;
    }

    color emission(const intersection& i) const override {
        if ( !i.m_front_face ) {
            return colors::black;
        }
        return tex->value( i.m_u, i.m_v, i.get_p() );
    }

    color average_radiance() const override {
        return m_emission_color;
    }

    bool is_delta() const override {
        return true;
    }

private:
    shared_ptr<texture> tex;
    color m_emission_color;
};

#endif //MAT_DIFFUSE_LIGHT_H
