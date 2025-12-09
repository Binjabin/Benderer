//
// Created by binjabin on 12/9/25.
//

#ifndef BENDERER_SKYBOX_H
#define BENDERER_SKYBOX_H
#include "../../utility/color.h"

class skybox {
public:
    virtual ~skybox() = default;

    virtual color sample_color(vec3 direction) const = 0;
};

#endif //BENDERER_SKYBOX_H