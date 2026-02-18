//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_METAL_H
#define MAT_METAL_H

class metal : public surface_material {
public:
    metal( const color& albedo, double fuzz ) : m_albedo( albedo ), m_fuzz( fuzz < 1 ? fuzz : 1 ) {
    }

    bool scatter( const ray& r_in, const surface_hit_rec& rec, surface_scatter_rec& srec ) const override {
        vec3 reflected = reflect( r_in.direction(), rec.get_normal() );
        reflected = unit_vector( reflected ) + ( m_fuzz * random_unit_vector() );

        srec.bsdf = get_bsdf(rec);
        srec.s_ray = ray( rec.get_p() + reflected * epsilon, reflected, r_in.time() );
        srec.is_spec = true;

        return true;
    }

    color get_bsdf(const surface_hit_rec &rec) const override {
        return m_albedo;
    }

private:
    color m_albedo;
    double m_fuzz;
};

#endif //MAT_METAL_H
