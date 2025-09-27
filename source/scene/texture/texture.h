//
// Created by binjabin on 7/8/25.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include "../../utility/benderer_stb_image.h"
#include "../../utility/perlin.h"
#include "../../structures/vec3.h"

class texture {
public:
    virtual ~texture() = default;
    virtual color value( double u, double v, const point3& p ) const = 0;
};

#endif //TEXTURE_H
