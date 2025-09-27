cmake_minimum_required(VERSION 3.31)
project(Benderer)

set(CMAKE_CXX_STANDARD 20)

add_executable(Benderer main.cpp
        vec3.h
        color.h
        ray.h
        hittable.h
        sphere.h
        hittable_list.h
        benderer.h
        interval.h
        camera.h
        material.h
        aabb.h
        bvh.h
        texture.h
        benderer_stb_image.h
        perlin.h
        quad.h
        constant_medium.h
        onb.h
        pdf.h)
