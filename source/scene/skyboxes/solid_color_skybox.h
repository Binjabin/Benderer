//
// Created by binjabin on 12/9/25.
//

#ifndef BENDERER_SOLID_COLOR_SKYBOX_H
#define BENDERER_SOLID_COLOR_SKYBOX_H
#include "skybox.h"

class solid_color_skybox : public skybox {
public:
    solid_color_skybox(const color& color) :
        m_color(color) {
    }

    color sample_color(vec3 direction) const override {
        return m_color;
    }

private:
    const color m_color;
};

#endif //BENDERER_SOLID_COLOR_SKYBOX_H