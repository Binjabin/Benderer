#include "benderer.h"
#include "image/image_info_library.h"
#include "integrators/is_path_tracer.h"

#include "scene/scene.h"
#include "scene/scene_library.h"
#include "scene/world.h"
#include "utility/time.h"
#include "image/writer/psix_writer.h"
#include "image/writer/png_writer.h"
#include "image/post/post_process.h"
#include "integrators/mis_medium_path_tracer.h"
#include "integrators/rr_medium_path_tracer.h"
#include "integrators/is_medium_path_tracer.h"
#include "integrators/simple_medium_path_tracer.h"
#include "integrators/simple_path_tracer.h"
#include "scene/hittables/surfaces/surface_tree.h"

int main(int argc, char** argv) {
    const std::string scene_name = argc > 1 ? argv[1] : "cornell_showcase";
    scene our_scene = scene_library::by_name(scene_name);

    our_scene.finalize();

    //auto info = image_info_library::ablation_1();
    //auto info = image_info_library::ablation_2();
    //auto info = image_info_library::ablation_3();
    auto info = image_info_library::ablation_4();
    //auto info = image_info_library::ablation_ref();

    int w = info.pixel_width();
    int h = info.pixel_height();
    post_process post = post_process(w, h);
    png_writer image_maker = png_writer(w, h);
    psix_writer preview_image_maker = psix_writer(w, h);

    std::string filename = "output_";
    filename += get_time_string();

    //auto itgr = simple_medium_path_tracer(info.max_depth());
    //auto itgr = is_medium_path_tracer(info.max_depth());
    //auto itgr = rr_medium_path_tracer(info.max_depth(), 8);
    //auto itgr = mis_medium_path_tracer(info.max_depth(), 8, 1);
    auto itgr = mis_medium_path_tracer(info.max_depth(), 8, 8);

    camera cam = our_scene.m_cam;
    world world = our_scene.m_world;

    //Put stuff in acceleration structure
    world.accelerate();

    our_scene.render( filename, info, itgr, post, preview_image_maker, image_maker);

    return 0;
}
