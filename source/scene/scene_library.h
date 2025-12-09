//
// Created by binjabin on 9/27/25.
//

#ifndef SCENE_LIBRARY_H
#define SCENE_LIBRARY_H
#include "scene.h"

#include "camera.h"
#include "hittables/hittable.h"
#include "hittables/hittable_list.h"
#include "hittables/transforms/translate.h"
#include "hittables/transforms/rotate_y.h"
#include "hittables/primatives/quad.h"
#include "hittables/primatives/sphere.h"

#include "material/material.h"
#include "material/materials/mat_lambertian.h"
#include "material/materials/mat_dielectric.h"
#include "material/materials/mat_diffuse_light.h"
#include "material/materials/mat_metal.h"

#include "texture/textures/tex_checker.h"
#include "texture/textures/tex_image.h"
#include "texture/textures/tex_noise.h"

class scene_library {
public:
    static scene checkered_spheres() {
        camera cam;
        cam.vfov = 20;
        cam.lookfrom = point3( 13, 2, 3 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        hittable_list world;
        auto checker = make_shared<checker_texture>( 0.32, color( .2, .3, .1 ), color( .9, .9, .9 ) );
        world.add( make_shared<sphere>( point3( 0, -10, 0 ), 10, make_shared<lambertian>( checker ) ) );
        world.add( make_shared<sphere>( point3( 0, 10, 0 ), 10, make_shared<lambertian>( checker ) ) );

        hittable_list lights;

        color background = color( 0.70, 0.80, 1.00 );

        return scene( cam, world, lights, background );
    }

    static scene earth() {
        camera cam;
        cam.vfov = 20;
        cam.lookfrom = point3( 4, 4, 12 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        hittable_list world;
        auto earth_texture = make_shared<image_texture>( "earthmap.jpg" );
        auto earth_surface = make_shared<lambertian>( earth_texture );
        auto globe = make_shared<sphere>( point3( 0, 0, 0 ), 2, earth_surface );
        world.add( globe );

        hittable_list lights;

        color background = color( 0.70, 0.80, 1.00 );

        return scene( cam, world, lights, background );
    }

    static scene random_balls() {
        camera cam;
        cam.vfov = 20;
        cam.lookfrom = point3( 13, 2, 3 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0.6;
        cam.focus_dist = 10.0;

        hittable_list world;
        auto check_tex = make_shared<checker_texture>( 0.2, color( .5, .5, .5 ), color( .9, .9, .9 ) );
        auto ground_material = make_shared<lambertian>( check_tex );
        world.add( make_shared<sphere>( point3( 0, -1000, -1 ), 1000, ground_material ) );
        for ( int a = -11; a < 11; a++ ) {
            for ( int b = -11; b < 11; b++ ) {
                auto choose_mat = random_double();
                point3 center( a + 0.9 * random_double(), 0.2, b + 0.9 * random_double() );

                if ( ( center - point3( 4, 0.2, 0 ) ).length() > 0.9 ) {
                    shared_ptr<material> sphere_material;

                    if ( choose_mat < 0.8 ) {
                        // diffuse
                        auto albedo = color::random() * color::random();
                        sphere_material = make_shared<lambertian>( albedo );

                        auto center2 = center + vec3( 0, random_double( 0, 0.5 ), 0 );
                        world.add( make_shared<sphere>( center, center2, 0.2, sphere_material ) );
                    }
                    else if ( choose_mat < 0.95 ) {
                        // metal
                        auto albedo = color::random( 0.5, 1 );
                        auto fuzz = random_double( 0, 0.5 );
                        sphere_material = make_shared<metal>( albedo, fuzz );
                        world.add( make_shared<sphere>( center, 0.2, sphere_material ) );
                    }
                    else {
                        // glass
                        sphere_material = make_shared<dielectric>( 1.5 );
                        world.add( make_shared<sphere>( center, 0.2, sphere_material ) );
                    }
                }
            }
        }
        auto material1 = make_shared<dielectric>( 1.5 );
        world.add( make_shared<sphere>( point3( 0, 1, 0 ), 1.0, material1 ) );
        auto material2 = make_shared<lambertian>( color( 0.4, 0.2, 0.1 ) );
        world.add( make_shared<sphere>( point3( -4, 1, 0 ), 1.0, material2 ) );
        auto material3 = make_shared<metal>( color( 0.7, 0.6, 0.5 ), 0.0 );
        world.add( make_shared<sphere>( point3( 4, 1, 0 ), 1.0, material3 ) );
        world = hittable_list( make_shared<bvh_node>( world ) );

        hittable_list lights;

        color background = color( 0.70, 0.80, 1.00 );

        return scene( cam, world, lights, background );
    }

    static scene perlin_spheres() {
        camera cam;
        cam.vfov = 20;
        cam.lookfrom = point3( 13, 2, 3 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        hittable_list world;
        auto pertext = make_shared<noise_texture>( 4 );
        world.add( make_shared<sphere>( point3( 0, -1000, 0 ), 1000, make_shared<lambertian>( pertext ) ) );
        world.add( make_shared<sphere>( point3( 0, 2, 0 ), 2, make_shared<lambertian>( pertext ) ) );

        hittable_list lights;

        color background = color( 0.70, 0.80, 1.00 );

        return scene( cam, world, lights, background );
    }

    static scene quads() {
        camera cam;
        cam.vfov = 80;
        cam.lookfrom = point3( 0, 0, 9 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        hittable_list world;
        // Materials
        auto left_red = make_shared<lambertian>( color( 1.0, 0.2, 0.2 ) );
        auto back_green = make_shared<lambertian>( color( 0.2, 1.0, 0.2 ) );
        auto right_blue = make_shared<lambertian>( color( 0.2, 0.2, 1.0 ) );
        auto upper_orange = make_shared<lambertian>( color( 1.0, 0.5, 0.0 ) );
        auto lower_teal = make_shared<lambertian>( color( 0.2, 0.8, 0.8 ) );
        // Quads
        world.add( make_shared<quad>( point3( -3, -2, 5 ), vec3( 0, 0, -4 ), vec3( 0, 4, 0 ), left_red ) );
        world.add( make_shared<quad>( point3( -2, -2, 0 ), vec3( 4, 0, 0 ), vec3( 0, 4, 0 ), back_green ) );
        world.add( make_shared<quad>( point3( 3, -2, 1 ), vec3( 0, 0, 4 ), vec3( 0, 4, 0 ), right_blue ) );
        world.add( make_shared<quad>( point3( -2, 3, 1 ), vec3( 4, 0, 0 ), vec3( 0, 0, 4 ), upper_orange ) );
        world.add( make_shared<quad>( point3( -2, -3, 5 ), vec3( 4, 0, 0 ), vec3( 0, 0, -4 ), lower_teal ) );

        hittable_list lights;

        color background = color( 0.70, 0.80, 1.00 );

        return scene( cam, world, lights, background );
    }

    static scene simple_light() {
        camera cam;
        cam.vfov = 20;
        cam.lookfrom = point3( 26, 3, 6 );
        cam.lookat = point3( 0, 2, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        hittable_list world;
        auto pertext = make_shared<noise_texture>( 4 );
        world.add( make_shared<sphere>( point3( 0, -1000, 0 ), 1000, make_shared<lambertian>( pertext ) ) );
        world.add( make_shared<sphere>( point3( 0, 2, 0 ), 2, make_shared<lambertian>( pertext ) ) );

        auto light_mat = make_shared<diffuse_light>( color( 4, 4, 4 ) );
        auto light_quad = make_shared<quad>( point3( 3, 1, -2 ), vec3( 2, 0, 0 ), vec3( 0, 2, 0 ), light_mat );

        world.add( light_quad );

        hittable_list lights;
        lights.add( light_quad );

        color background = color( 0, 0, 0 );

        return scene( cam, world, lights, background );
    }

    static scene cornell_box() {
        camera cam;
        cam.vfov = 40;
        cam.lookfrom = point3( 278, 278, -800 );
        cam.lookat = point3( 278, 278, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        hittable_list world;
        auto red = make_shared<lambertian>( color( .65, .05, .05 ) );
        auto white = make_shared<lambertian>( color( .73, .73, .73 ) );
        auto green = make_shared<lambertian>( color( .12, .45, .15 ) );
        auto aluminum = make_shared<metal>( color( 0.8, 0.85, 0.88 ), 0.01 );
        world.add( make_shared<quad>( point3( 555, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), green ) );
        world.add( make_shared<quad>( point3( 0, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), red ) );
        world.add( make_shared<quad>( point3( 0, 0, 0 ), vec3( 555, 0, 0 ), vec3( 0, 0, 555 ), white ) );
        world.add( make_shared<quad>( point3( 555, 555, 555 ), vec3( -555, 0, 0 ), vec3( 0, 0, -555 ), white ) );
        world.add( make_shared<quad>( point3( 0, 0, 555 ), vec3( 555, 0, 0 ), vec3( 0, 555, 0 ), white ) );
        shared_ptr<hittable> box1 = box( point3( 0, 0, 0 ), point3( 165, 330, 165 ), aluminum );
        box1 = make_shared<rotate_y>( box1, 15 );
        box1 = make_shared<translate>( box1, vec3( 265, 0, 295 ) );
        world.add( box1 );
        shared_ptr<hittable> box2 = box( point3( 0, 0, 0 ), point3( 165, 165, 165 ), white );
        box2 = make_shared<rotate_y>( box2, -18 );
        box2 = make_shared<translate>( box2, vec3( 130, 0, 65 ) );
        world.add( box2 );

        auto light_mat = make_shared<diffuse_light>( color( 15, 15, 15 ) );
        auto light = make_shared<quad>( point3( 343, 554, 332 ), vec3( -130, 0, 0 ), vec3( 0, 0, -105 ), light_mat );

        world.add( light );

        hittable_list lights;
        lights.add( light );

        color background = color( 0, 0, 0 );

        return scene( cam, world, lights, background );
    }

    static scene cornell_ball() {
        camera cam;
        cam.vfov = 40;
        cam.lookfrom = point3( 278, 278, -800 );
        cam.lookat = point3( 278, 278, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        hittable_list world;
        auto red = make_shared<lambertian>( color( .65, .05, .05 ) );
        auto white = make_shared<lambertian>( color( .73, .73, .73 ) );
        auto green = make_shared<lambertian>( color( .12, .45, .15 ) );
        auto aluminum = make_shared<metal>( color( 0.8, 0.85, 0.88 ), 0.01 );
        world.add( make_shared<quad>( point3( 555, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), green ) );
        world.add( make_shared<quad>( point3( 0, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), red ) );
        world.add( make_shared<quad>( point3( 0, 0, 0 ), vec3( 555, 0, 0 ), vec3( 0, 0, 555 ), white ) );
        world.add( make_shared<quad>( point3( 555, 555, 555 ), vec3( -555, 0, 0 ), vec3( 0, 0, -555 ), white ) );
        world.add( make_shared<quad>( point3( 0, 0, 555 ), vec3( 555, 0, 0 ), vec3( 0, 555, 0 ), white ) );
        // Box
        shared_ptr<hittable> box1 = box( point3( 0, 0, 0 ), point3( 165, 330, 165 ), white );
        box1 = make_shared<rotate_y>( box1, 15 );
        box1 = make_shared<translate>( box1, vec3( 265, 0, 295 ) );
        world.add( box1 );
        // Glass Sphere
        auto glass_mat = make_shared<dielectric>( 1.5 );
        auto glass_ball = make_shared<sphere>( point3( 190, 90, 190 ), 90, glass_mat );
        world.add( glass_ball );

        // Light Sources
        auto light_mat = make_shared<diffuse_light>( color( 1500, 1500, 1500 ) );

        auto light = make_shared<quad>( point3( 343, 554, 332 ), vec3( -130, 0, 0 ), vec3( 0, 0, -105 ), light_mat );
        // Light
        world.add( light );
        hittable_list lights;
        lights.add( light );

        //Others to sample
        lights.add( glass_ball );

        //color background = color( 0.70, 0.80, 1.00 );
        color background = color( 0, 0, 0 );

        return scene( cam, world, lights, background );
    }
};

#endif //SCENE_LIBRARY_H
