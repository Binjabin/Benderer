#include "benderer.h"
#include "scene/hittables/bvh.h"


#include "scene/scene.h"
#include "scene/scene_library.h"

int main() {
    freopen( "output.ppm", "w", stdout );

    const int chosen_scene = 8;
    scene our_scene = get_scene( chosen_scene );
    our_scene.render();

    return 0;
}
