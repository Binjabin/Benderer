#include "benderer.h"
#include "image/image_info_library.h"
#include "integrators/mips_model.h"
#include "integrators/rtw_model.h"

#include "scene/hittables/bvh.h"
#include "scene/scene.h"
#include "scene/scene_library.h"

int main() {
    freopen( "output.ppm", "w", stdout );

    scene our_scene = scene_library::simple_light();
    our_scene.finalize();
    image_info info = image_info_library::preview_sol();
    //auto itgr = rtw_model();
    auto itgr = mips_model();
    our_scene.render(info, itgr);

    return 0;
}
