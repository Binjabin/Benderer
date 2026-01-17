#include "benderer.h"
#include "image/image_info_library.h"
#include "integrators/simple_path_tracer.h"

#include "scene/hittables/bvh.h"
#include "scene/scene.h"
#include "scene/scene_library.h"
#include "scene/world.h"

int main() {
    freopen( "output.ppm", "w", stdout );

    scene our_scene = scene_library::cornell_box();
    our_scene.finalize();
    image_info info = image_info_library::micro_ultra();
    //auto itgr = rtw_model();
    //auto itgr = mips_model(info.max_depth(), 2, 16);
    auto itgr = simple_path_tracer(info.max_depth());

    camera cam = our_scene.m_cam;
    world world = our_scene.m_world;
    our_scene.render(info, itgr);

    return 0;
}
