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
        source/scene/hittables/primatives/constant_medium.h
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
)
