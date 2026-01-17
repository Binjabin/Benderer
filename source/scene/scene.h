//
// Created by binjabin on 9/27/25.
//

#ifndef SCENE_H
#define SCENE_H
#include "camera.h"
#include "world.h"
#include "hittables/hittable.h"
#include "hittables/hittable_list.h"
#include "skyboxes/skybox.h"

class scene {
public:
    ~scene() = default;

    scene( camera& cam, const hittable_list& world, const hittable_list& lights, const shared_ptr<skybox> skybox)
        : m_cam( cam ), m_world( world, lights, skybox ){
    }

    void finalize() {
        m_world.m_geometry.compute_properties();
        m_world.m_lights.compute_properties();
        m_world.m_lights.set_explicit_light(true);
    }

    void render(const image_info& info, const integrator& itgr) {
        m_cam.render(m_world, info, itgr);
    }

    camera m_cam;
    world m_world;
};

#endif //SCENE_H
