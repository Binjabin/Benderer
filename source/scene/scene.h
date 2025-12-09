//
// Created by binjabin on 9/27/25.
//

#ifndef SCENE_H
#define SCENE_H
#include "camera.h"
#include "hittables/hittable_list.h"

class scene {
public:
    ~scene() = default;

    scene( camera& cam, const hittable_list& world, const hittable_list& lights, const color& background )
        : m_cam( cam ), m_world( world ), m_lights( lights ), m_background( background ) {
    }

    void render( const image_info& info, const integrator& itgr ) {
        m_cam.render( m_world, m_lights, m_background, info, itgr);
    }

private:
    camera m_cam;
    const hittable_list m_world;
    const hittable_list m_lights;
    const color m_background;
};

#endif //SCENE_H
