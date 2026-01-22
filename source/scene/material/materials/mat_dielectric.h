//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_DIELECTRIC_H
#define MAT_DIELECTRIC_H

class dielectric : public material {
public:
    dielectric( double refraction_index ) : refraction_index( refraction_index ) {
    }

    bool scatter( const ray& r_in, const surface_hit& rec, scatter_record& srec ) const override {
        srec.attenuation = color( 1.0, 1.0, 1.0 );
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;

        double ri = rec.get_front_face() ? ( 1.0 / refraction_index ) : refraction_index;

        vec3 unit_direction = unit_vector( r_in.direction() );
        double cos_theta = std::fmin( dot( -unit_direction, rec.get_normal() ), 1.0 );
        double sin_theta = std::sqrt( 1.0 - cos_theta * cos_theta );

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        double r = reflectance( cos_theta, ri );

        if ( cannot_refract || random_double() < r) {
            direction = reflect( unit_direction, rec.get_normal() );
        }
        else {
            direction = refract( unit_direction, rec.get_normal(), ri );
        }

        srec.skip_pdf_ray = ray( rec.get_p() + direction * epsilon, direction, r_in.time() );

        return true;
    }

    color bsdf(vec3 d_in, const surface_hit &rec, const vec3 &r_out) override {
        //Don't use monte carlo here
        return color(0, 0, 0);
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
