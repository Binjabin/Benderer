//
// Created by binjabin on 9/27/25.
//

#ifndef SCENE_H
#define SCENE_H
#include "camera.h"
#include "world.h"
#include "../image/writer/image_writer.h"
#include "../image/post/post_process.h"
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

    void render(const std::string& filename, const image_info& info, const integrator& itgr, post_process& post, const image_writer& preview_writer, const image_writer& result_writer) {
        m_cam.render(filename, m_world, info, itgr, post, preview_writer, result_writer);
    }

    camera m_cam;
    world m_world;
};

#endif //SCENE_H
