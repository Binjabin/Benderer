//
// Created by binjabin on 7/8/25.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include "../../utility/perlin.h"
#include "../../structures/vec3.h"

class texture {
public:
    virtual ~texture() = default;
    color value (const intersection& isect) {
        return value(isect.m_u, isect.m_v, isect.get_p());
    }
    virtual color value( double u, double v, const point3& p ) const = 0;
    virtual color get_base_color() const {
        return value(0, 0, point3(0,0,0));
    };
};

#endif //TEXTURE_H
