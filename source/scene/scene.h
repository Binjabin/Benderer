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
        : cam( cam ), world( world ), lights( lights ) {
    }

    void render() {
        cam.render( world, lights );
    }

private:
    camera cam;
    hittable_list world;
    hittable_list lights;
};

#endif //SCENE_H
