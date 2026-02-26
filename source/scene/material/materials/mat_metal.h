//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_METAL_H
#define MAT_METAL_H



class metal : public surface_material {
public:
    metal( const color& albedo, double fuzz ) : m_albedo( albedo ), m_fuzz( fuzz < 1 ? fuzz : 1 ) {
    }

    color bsdf(const intersection& i, const vec3& in, const vec3& out) override {
        return m_albedo;
    }

    double pdf(const intersection& i, const vec3& in, const vec3& out) override {
        return uninit;
    }

    bool scatter(const intersection& i, const vec3& in, surface_scatter_rec& srec) override {
        vec3 unit_in = unit_vector( in );
        vec3 reflected = reflect( unit_in, i.m_normal );
        reflected = unit_vector( reflected + ( m_fuzz * random_unit_vector()) );

        if (dot(reflected, i.m_normal) <= 0.0) return false;

        srec.s_ray = ray( i.get_p() + reflected * epsilon, reflected, i.get_time() );
        srec.w_pdf = uninit;
        srec.bsdf = m_albedo;
        srec.is_delta = true;

        return true;
    }

    color emission(const intersection& i) const override {
        return colors::black;
    }

    color get_radiance() const override {
        return colors::black;
    }

    bool is_delta() const override {
        return true;
    }

private:
    color m_albedo;
    double m_fuzz;
};

#endif //MAT_METAL_H
