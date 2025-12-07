#include "benderer.h"
#include "image/image_info_library.h"

#include "scene/hittables/bvh.h"
#include "scene/scene.h"
#include "scene/scene_library.h"

int main() {
    freopen( "output.ppm", "w", stdout );

    scene our_scene = scene_library::earth();
    image_info info = image_info_library::low();
    our_scene.render( info );

    return 0;
}
