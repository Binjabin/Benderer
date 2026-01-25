//
// Created by binjabin on 1/17/26.
//

#ifndef BENDERER_WORLD_H
#define BENDERER_WORLD_H

#include "hittables/hittable.h"
#include "hittables/mediums/medium.h"
#include "hittables/surfaces/surface_list.h"
#include "skyboxes/skybox.h"

class world {
public:
    ~world() = default;

    world( shared_ptr<surface> surfaces, shared_ptr<medium> mediums, shared_ptr<surface> lights, const shared_ptr<skybox> skybox)
        : m_surfaces( surfaces ), m_mediums(mediums), m_lights( lights ), m_sky( skybox ) {
    }

    shared_ptr<surface> m_surfaces;
    shared_ptr<medium> m_mediums;
    shared_ptr<surface> m_lights;
    const shared_ptr<skybox> m_sky;
};

#endif //BENDERER_WORLD_H