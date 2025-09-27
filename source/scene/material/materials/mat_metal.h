//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_METAL_H
#define MAT_METAL_H

class metal : public material {
public:
    metal( const color& albedo, double fuzz ) : albedo( albedo ), fuzz( fuzz < 1 ? fuzz : 1 ) {
    }

    bool scatter( const ray& r_in, const hit_record& rec, scatter_record& srec ) const override {
        vec3 reflected = reflect( r_in.direction(), rec.normal );
        reflected = unit_vector( reflected ) + ( fuzz * random_unit_vector() );

        srec.attenuation = albedo;
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;
        srec.skip_pdf_ray = ray( rec.p, reflected, r_in.time() );

        return true;
    }

private:
    color albedo;
    double fuzz;
};

#endif //MAT_METAL_H
