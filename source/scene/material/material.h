//
// Created by binjabin on 6/29/25.
//

#ifndef MATERIAL_H
#define MATERIAL_H

#include "../hittables/hittable.h"
#include "../../structures/pdf.h"
#include "../../structures/vec3.h"
#include "../../structures/scatter_record.h"

class material {
public:
    virtual ~material() = default;

    virtual color emitted( const ray& r_in, const hit_record& rec, double u, double v, const point3& p ) const {
        return color( 0, 0, 0 );
    }

    virtual bool scatter( const ray& r_in, const hit_record& rec, scatter_record& srec ) const {
        return false;
    }

    virtual double scattering_pdf( const ray& r_in, const hit_record& rec, const ray& scattered ) const {
        return 0;
    }
};

#endif //MATERIAL_H
