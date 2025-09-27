//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_ISOTROPIC_H
#define MAT_ISOTROPIC_H

class isotropic : public material {
public:
    isotropic( const color& albedo ) : tex( make_shared<solid_color>( albedo ) ) {
    }

    isotropic( shared_ptr<texture> tex ) : tex( tex ) {
    }

    bool scatter( const ray& r_in, const hit_record& rec, scatter_record& srec ) const override {
        srec.attenuation = tex->value( rec.u, rec.v, rec.p );
        srec.pdf_ptr = make_shared<sphere_pdf>();
        srec.skip_pdf = false;
        return true;
    }

    double scattering_pdf( const ray& r_in, const hit_record& rec, const ray& scattered ) const override {
        return 1 / ( 4 * pi );
    }

private:
    shared_ptr<texture> tex;
};

#endif //MAT_ISOTROPIC_H
