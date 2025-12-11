//
// Created by binjabin on 9/27/25.
//

#ifndef SCENE_H
#define SCENE_H
#include "camera.h"
#include "hittables/hittable.h"
#include "hittables/hittable_list.h"

class scene {
public:
    ~scene() = default;

    scene( camera& cam, const hittable_list& world, const hittable_list& lights, const shared_ptr<skybox> skybox)
        : m_cam( cam ), m_world( world ), m_lights( lights ), m_sky( skybox ) {
    }

    void render( const image_info& info, const integrator& itgr ) {
        m_cam.render( m_world, m_lights, m_sky, info, itgr);
    }

    void finalize() {
        m_world.compute_properties();
        m_lights.compute_properties();
        m_lights.set_explicit_light(true);
    }

private:
    camera m_cam;
    hittable_list m_world;
    hittable_list m_lights;
    const shared_ptr<skybox> m_sky;
};

#endif //SCENE_H
