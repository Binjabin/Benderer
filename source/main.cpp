#include "benderer.h"
#include "image/image_info_library.h"
#include "integrators/simple_path_tracer.h"

#include "scene/scene.h"
#include "scene/scene_library.h"
#include "scene/world.h"
#include "utility/time.h"
#include "image/writer/psix_writer.h"
#include "image/writer/png_writer.h"
#include "image/post/post_process.h"
#include "integrators/mis_medium_path_tracer.h"
#include "integrators/rr_medium_path_tracer.h"
#include "integrators/simple_medium_path_tracer.h"
#include "scene/hittables/surfaces/surface_tree.h"

int main() {

    scene our_scene = scene_library::cornell_box();
    our_scene.finalize();
    image_info info = image_info_library::small_high();
    //auto itgr = rtw_model();
    //auto itgr = mips_model(info.max_depth(), 2, 16);
    //auto itgr = simple_medium_path_tracer(info.max_depth());
    int rr_depth = (info.max_depth() * 3) / 4;
    //auto itgr = rr_medium_path_tracer(info.max_depth(), rr_depth);
    auto itgr = mis_medium_path_tracer(info.max_depth(), rr_depth, 5);

    camera cam = our_scene.m_cam;
    world world = our_scene.m_world;

    //Put stuff in acceleration structure
    world.accelerate();

    int w = info.pixel_width();
    int h = info.pixel_height();
    post_process post = post_process(w, h);
    png_writer image_maker = png_writer(w, h);
    psix_writer preview_image_maker = psix_writer(w, h);

    std::string filename = "output_";
    filename += get_time_string();

    our_scene.render( filename, info, itgr, post, preview_image_maker, image_maker);

    //Close Window
    std::system("pkill feh");

    return 0;
}
