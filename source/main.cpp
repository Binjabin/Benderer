#include "benderer.h"
#include "image/image_info_library.h"
#include "integrators/simple_path_tracer.h"

#include "scene/scene.h"
#include "scene/scene_library.h"
#include "scene/world.h"
#include "utility/time.h"
#include "image/writer/png_writer.h"
#include "image/post/post_process.h"

int main() {
    freopen( "output.ppm", "w", stdout );

    scene our_scene = scene_library::cornell_ball();
    our_scene.finalize();
    image_info info = image_info_library::micro_sol();
    //auto itgr = rtw_model();
    //auto itgr = mips_model(info.max_depth(), 2, 16);
    auto itgr = simple_path_tracer(info.max_depth());

    camera cam = our_scene.m_cam;
    world world = our_scene.m_world;

    int w = info.pixel_width();
    int h = info.pixel_height();
    post_process post = post_process(w, h);
    png_writer image_maker = png_writer(w, h);

    std::string filename = "output_";
    filename += get_time_string();
    filename += ".png";
    our_scene.render( filename.data(), info, itgr, post, image_maker);

    return 0;
}
