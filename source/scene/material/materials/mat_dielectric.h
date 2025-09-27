//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_DIELECTRIC_H
#define MAT_DIELECTRIC_H

class dielectric : public material {
public:
    dielectric( double refraction_index ) : refraction_index( refraction_index ) {
    }

    bool scatter( const ray& r_in, const hit_record& rec, scatter_record& srec ) const override {
        srec.attenuation = color( 1.0, 1.0, 1.0 );
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;

        double ri = rec.front_face ? ( 1.0 / refraction_index ) : refraction_index;

        vec3 unit_direction = unit_vector( r_in.direction() );
        double cos_theta = std::fmin( dot( -unit_direction, rec.normal ), 1.0 );
        double sin_theta = std::sqrt( 1.0 - cos_theta * cos_theta );

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if ( cannot_refract ) {
            direction = reflect( unit_direction, rec.normal );
        }
        else {
            direction = refract( unit_direction, rec.normal, ri );
        }

        srec.skip_pdf_ray = ray( rec.p, direction, r_in.time() );

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
