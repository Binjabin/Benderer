//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_DIELECTRIC_H
#define MAT_DIELECTRIC_H

class dielectric : public surface_material {
public:
    dielectric( double refraction_index ) : refraction_index( refraction_index ) {
    }

    color bsdf(const intersection &i, const vec3 &in, const vec3 &out) override {
        return color(0, 0, 0);
    }

    double pdf(const intersection &i, const vec3 &in, const vec3 &out) override {
        return uninit;
    }

    bool scatter_is(const intersection& i, const vec3& in, surface_scatter_rec& srec) override {
        return scatter(i, in, srec);
    } 

    bool scatter(const intersection& i, const vec3& in, surface_scatter_rec& srec) override {
        srec.bsdf = color( 1.0, 1.0, 1.0 );

        double ri = i.m_front_face ? ( 1.0 / refraction_index ) : refraction_index;

        vec3 unit_direction = unit_vector( in );
        double cos_theta = std::fmin( dot( -unit_direction, i.m_normal ), 1.0 );
        double sin_theta = std::sqrt( 1.0 - cos_theta * cos_theta );

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        double r = reflectance( cos_theta, ri );

        if ( cannot_refract || random_double() < r) {
            direction = reflect( unit_direction, i.m_normal );
        }
        else {
            direction = refract( unit_direction, i.m_normal, ri );
        }

        srec.s_ray = ray( i.get_p() + direction * epsilon, direction, i.get_time() );

        srec.is_delta = true;

        return true;
    }



    color emission(const intersection& i) const override {
        return color(0, 0, 0);
    }

    color get_radiance() const override {
        return colors::black;
    }

    bool is_delta() const override {
        return true;
    }

private:
    double refraction_index;

    static double reflectance( double cosine, double refraction_index ) {
        auto r0 = ( 1 - refraction_index ) / ( 1 + refraction_index );
        r0 = r0 * r0;
        return r0 + ( 1 - r0 ) * std::pow( ( 1 - cosine ), 5 );
    }
};

#endif //MAT_DIELECTRIC_H
