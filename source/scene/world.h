//
// Created by binjabin on 1/17/26.
//

#ifndef BENDERER_WORLD_H
#define BENDERER_WORLD_H

#include "hittables/hittable.h"
#include "hittables/hittable_list.h"
#include "skyboxes/skybox.h"

class world {
public:
    ~world() = default;

    world( const hittable_list& geometry, const hittable_list& lights, const shared_ptr<skybox> skybox)
        : m_geometry( geometry ), m_lights( lights ), m_sky( skybox ) {
    }

    hittable_list m_geometry;
    hittable_list m_lights;
    const shared_ptr<skybox> m_sky;
};

#endif //BENDERER_WORLD_H