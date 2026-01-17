cmake_minimum_required(VERSION 3.31)
project(Benderer)

set(CMAKE_CXX_STANDARD 20)

add_executable(Benderer source/main.cpp
        source/structures/vec3.h
        source/utility/color.h
        source/structures/ray.h
        source/scene/hittables/hittable.h
        source/scene/hittables/primatives/sphere.h
        source/scene/hittables/hittable_list.h
        source/benderer.h
        source/structures/interval.h
        source/scene/camera.h
        source/scene/material/material.h
        source/structures/aabb.h
        source/scene/hittables/bvh.h
        source/scene/texture/texture.h
        source/utility/benderer_stb_image.h
        source/utility/perlin.h
        source/scene/hittables/primatives/quad.h
        source/scene/hittables/constant_medium.h
        source/structures/onb.h
        source/structures/pdf.h
        source/scene/texture/textures/tex_solid_colour.h
        source/scene/texture/textures/tex_checker.h
        source/scene/texture/textures/tex_image.h
        source/scene/texture/textures/tex_noise.h
        source/scene/material/materials/mat_lambertian.h
        source/scene/material/materials/mat_isotropic.h
        source/scene/material/materials/mat_dielectric.h
        source/scene/material/materials/mat_metal.h
        source/scene/material/materials/mat_diffuse_light.h
        source/scene/scene.h
        source/scene/scene_library.h
        source/image/image_info.h
        source/structures/scatter_record.h
        source/image/image_info_library.h
        source/structures/light_sample.h
        source/scene/hittables/primatives/primitive.h
        source/scene/hittables/transforms/transform.h
        source/scene/hittables/transforms/translate.h
        source/scene/hittables/transforms/rotate_y.h
        unused_source/rtw_model.h
        source/integrators/integrator.h
        source/scene/skyboxes/skybox.h
        source/scene/skyboxes/gradient_skybox.h
        source/scene/skyboxes/solid_color_skybox.h
        unused_source/mips_model.h
        unused_source/one_step_shadow.h
        source/structures/path_state.h
        source/structures/path_result.h
        unused_source/mips_model_2.h
        source/integrators/simple_path_tracer.h
        source/scene/world.h
)
