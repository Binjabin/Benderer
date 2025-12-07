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

    scene( camera cam, hittable_list world, hittable_list lights )
        : m_cam( cam ), m_world( world ), m_lights( lights ) {
    }

    void render( image_info info ) {
        m_cam.render( m_world, m_lights, info );
    }

private:
    camera m_cam;
    hittable_list m_world;
    hittable_list m_lights;
};

#endif //SCENE_H
