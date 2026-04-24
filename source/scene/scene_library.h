//
// Created by binjabin on 9/27/25.
//

#ifndef SCENE_LIBRARY_H
#define SCENE_LIBRARY_H
#include "scene.h"

#include "camera.h"
#include "hittables/hittable.h"
#include "hittables/surfaces/primitive_surface.h"
#include "object_library.h"
#include "hittables/mediums/medium_list.h"
#include "hittables/surfaces/surface_tree.h"

#include "transforms/translate.h"
#include "transforms/rotate_y.h"

#include "shapes/flats/quad.h"
#include "shapes/solids/sphere.h"

#include "material/surface_material.h"
#include "material/surface materials/mat_lambertian.h"
#include "material/surface materials/mat_dielectric.h"
#include "material/surface materials/emissive.h"
#include "material/surface materials/mat_metal.h"
#include "material/medium materials/medium_mat_constant.h"
#include "material/medium materials/medium_mat_hg_constant.h"
#include "material/medium materials/medium_mat_grid.h"
#include "../structures/density_grid.h"
#include "../utility/perlin.h"
#include "skyboxes/gradient_skybox.h"
#include "skyboxes/solid_color_skybox.h"

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

        surface_list surfaces;
        auto checker = make_shared<checker_texture>( 0.32, colors::n_teal, colors::n_white );

        surfaces.add( object_library::make_sphere( point3( 0, -10, 0 ), 10, make_shared<lambertian>( checker ) ) );
        surfaces.add( object_library::make_sphere( point3( 0, 10, 0 ), 10, make_shared<lambertian>( checker ) ) );

        medium_list mediums;

        surface_list surface_lights;
        medium_list medium_lights;

        auto const skybox = make_shared<gradient_skybox>(colors::n_white, colors::sky);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene earth() {
        camera cam;
        cam.vfov = 20;
        cam.lookfrom = point3( 4, 4, 12 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        auto earth_texture = make_shared<image_texture>( "earthmap.jpg" );
        auto earth_surface = make_shared<lambertian>( earth_texture );
        auto globe = object_library::make_sphere( point3( 0, 0, 0 ), 2, earth_surface );
        surfaces.add( globe );

        medium_list mediums;

        surface_list surface_lights;
        medium_list medium_lights;

        auto const skybox = make_shared<gradient_skybox>(colors::n_white, colors::sky);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene random_balls() {
        camera cam;
        cam.vfov = 20;
        cam.lookfrom = point3( 13, 2, 3 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0.6;
        cam.focus_dist = 10.0;

        surface_list surfaces;
        auto check_tex = make_shared<checker_texture>( 0.2, colors::gray, colors::n_white );
        auto ground_material = make_shared<lambertian>( check_tex );
        surfaces.add( object_library::make_sphere( point3( 0, -1000, -1 ), 1000, ground_material ) );
        for ( int a = -11; a < 11; a++ ) {
            for ( int b = -11; b < 11; b++ ) {
                auto choose_mat = random_double();
                point3 center( a + 0.9 * random_double(), 0.2, b + 0.9 * random_double() );

                if ( ( center - point3( 4, 0.2, 0 ) ).length() > 0.9 ) {
                    shared_ptr<surface_material> sphere_material;

                    if ( choose_mat < 0.8 ) {
                        // diffuse
                        auto albedo = color::random() * color::random();
                        sphere_material = make_shared<lambertian>( albedo );

                        // Use static spheres only per instruction
                        surfaces.add( object_library::make_sphere( center, 0.2, sphere_material ) );
                    }
                    else if ( choose_mat < 0.95 ) {
                        // metal
                        auto albedo = color::random( 0.5, 1 );
                        auto fuzz = random_double( 0, 0.5 );
                        sphere_material = make_shared<metal>( albedo, fuzz );
                        surfaces.add( object_library::make_sphere( center, 0.2, sphere_material ) );
                    }
                    else {
                        // glass
                        sphere_material = make_shared<dielectric>( 1.5 );
                        surfaces.add( object_library::make_sphere( center, 0.2, sphere_material ) );
                    }
                }
            }
        }
        auto material1 = make_shared<dielectric>( 1.5 );
        surfaces.add( object_library::make_sphere( point3( 0, 1, 0 ), 1.0, material1 ) );
        auto material2 = make_shared<lambertian>( color( 0.4, 0.2, 0.1 ) );
        surfaces.add( object_library::make_sphere( point3( -4, 1, 0 ), 1.0, material2 ) );
        auto material3 = make_shared<metal>( color( 0.7, 0.6, 0.5 ), 0.0 );
        surfaces.add( object_library::make_sphere( point3( 4, 1, 0 ), 1.0, material3 ) );
        surfaces = surface_list( make_shared<surface_list>( surfaces ) );

        medium_list mediums;

        surface_list surface_lights;
        medium_list medium_lights;

        auto const skybox = make_shared<gradient_skybox>(colors::n_white, colors::sky);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene perlin_spheres() {
        camera cam;
        cam.vfov = 20;
        cam.lookfrom = point3( 13, 2, 3 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        auto pertext = make_shared<noise_texture>( 4 );
        surfaces.add( object_library::make_sphere( point3( 0, -1000, 0 ), 1000, make_shared<lambertian>( pertext ) ) );
        surfaces.add( object_library::make_sphere( point3( 0, 2, 0 ), 2, make_shared<lambertian>( pertext ) ) );

        medium_list mediums;

        surface_list surface_lights;
        medium_list medium_lights;

        auto const skybox = make_shared<gradient_skybox>(colors::n_white, colors::sky);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene quads() {
        camera cam;
        cam.vfov = 80;
        cam.lookfrom = point3( 0, 0, 9 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        // Materials
        auto left_red = make_shared<lambertian>( colors::n_red );
        auto back_green = make_shared<lambertian>( colors::n_green );
        auto right_blue = make_shared<lambertian>( colors::n_blue );
        auto upper_orange = make_shared<lambertian>( colors::n_orange );
        auto lower_teal = make_shared<lambertian>( colors::n_teal );
        // Quads
        surfaces.add( object_library::make_quad( point3( -3, -2, 5 ), vec3( 0, 0, -4 ), vec3( 0, 4, 0 ), left_red ) );
        surfaces.add( object_library::make_quad( point3( -2, -2, 0 ), vec3( 4, 0, 0 ), vec3( 0, 4, 0 ), back_green ) );
        surfaces.add( object_library::make_quad( point3( 3, -2, 1 ), vec3( 0, 0, 4 ), vec3( 0, 4, 0 ), right_blue ) );
        surfaces.add( object_library::make_quad( point3( -2, 3, 1 ), vec3( 4, 0, 0 ), vec3( 0, 0, 4 ), upper_orange ) );
        surfaces.add( object_library::make_quad( point3( -2, -3, 5 ), vec3( 4, 0, 0 ), vec3( 0, 0, -4 ), lower_teal ) );

        medium_list mediums;

        surface_list surface_lights;
        medium_list medium_lights;

        auto const skybox = make_shared<gradient_skybox>(colors::n_white, colors::sky);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene simple_light() {
        camera cam;
        cam.vfov = 20;
        cam.lookfrom = point3( 26, 3, 6 );
        cam.lookat = point3( 0, 2, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        auto pertext = make_shared<noise_texture>( 4 );
        surfaces.add( object_library::make_sphere( point3( 0, -1000, 0 ), 1000, make_shared<lambertian>( pertext ) ) );
        surfaces.add( object_library::make_sphere( point3( 0, 2, 0 ), 2, make_shared<lambertian>( pertext ) ) );

        auto light_mat = make_shared<emissive>( colors::dim_light );
        auto light_quad = object_library::make_quad( point3( 3, 1, -2 ), vec3( 2, 0, 0 ), vec3( 0, 2, 0 ), light_mat );

        surfaces.add( light_quad );

        medium_list mediums;

        surface_list surface_lights;
        surface_lights.add( light_quad );

        medium_list medium_lights;

        auto const skybox = make_shared<solid_color_skybox>(colors::black);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene cornell_box() {
        camera cam;
        cam.vfov = 40;
        cam.lookfrom = point3( 278, 278, -800 );
        cam.lookat = point3( 278, 278, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        auto red = make_shared<lambertian>( colors::n_red );
        auto white = make_shared<lambertian>( colors::n_white );
        auto green = make_shared<lambertian>( colors::n_green );
        auto aluminum = make_shared<metal>( colors::n_orange, 0.01 );
        surfaces.add( object_library::make_quad( point3( 555, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), green ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), red ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 555, 0, 0 ), vec3( 0, 0, 555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 555, 555, 555 ), vec3( -555, 0, 0 ), vec3( 0, 0, -555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 555 ), vec3( 555, 0, 0 ), vec3( 0, 555, 0 ), white ) );
        shared_ptr<surface> box1 = object_library::make_box( point3( 0, 0, 0 ), point3( 165, 330, 165 ), aluminum );
        box1 = object_library::make_rotate( box1, 20 );
        box1 = object_library::make_translate( box1, vec3( 265, 0, 295 ) );
        surfaces.add( box1 );
        shared_ptr<surface> box2 = object_library::make_box( point3( 0, 0, 0 ), point3( 165, 165, 165 ), white );
        box2 = object_library::make_rotate( box2, -18 );
        box2 = object_library::make_translate( box2, vec3( 130, 0, 65 ) );
        surfaces.add( box2 );

        auto light_mat = make_shared<emissive>( colors::bright_light );
        auto light = object_library::make_quad( point3( 343, 554, 332 ), vec3( -130, 0, 0 ), vec3( 0, 0, -105 ), light_mat );

        surfaces.add( light );

        medium_list mediums;

        surface_list surface_lights;
        surface_lights.add( light );

        medium_list medium_lights;

        auto const skybox = make_shared<solid_color_skybox>(colors::black);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene cornell_blue_ball() {
        camera cam;
        cam.vfov = 40;
        cam.lookfrom = point3( 278, 278, -800 );
        cam.lookat = point3( 278, 278, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        auto red = make_shared<lambertian>( colors::n_red );
        auto white = make_shared<lambertian>( colors::n_white );
        auto green = make_shared<lambertian>( colors::n_green );
        auto aluminum = make_shared<metal>( colors::metal_grey, 0.01 );
        surfaces.add( object_library::make_quad( point3( 555, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), green ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), red ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 555, 0, 0 ), vec3( 0, 0, 555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 555, 555, 555 ), vec3( -555, 0, 0 ), vec3( 0, 0, -555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 555 ), vec3( 555, 0, 0 ), vec3( 0, 555, 0 ), white ) );
        // Box
        shared_ptr<surface> box1 = object_library::make_box( point3( 0, 0, 0 ), point3( 165, 330, 165 ), white );
        box1 = object_library::make_rotate( box1, 15 );
        box1 = object_library::make_translate( box1, vec3( 265, 0, 295 ) );
        surfaces.add( box1 );
        // Glass Sphere
        auto glass_mat = make_shared<dielectric>( 1.5 );
        auto glass_ball = object_library::make_sphere( point3( 190, 90, 190 ), 90, glass_mat );
        surfaces.add( glass_ball );

        // Light Sources
        auto light_mat = make_shared<emissive>( colors::bright_light );

        auto light = object_library::make_quad( point3( 343, 554, 332 ), vec3( -130, 0, 0 ), vec3( 0, 0, -105 ), light_mat );
        // Light
        surfaces.add( light );

        medium_list mediums;
        auto frosted_mat = make_shared<medium_mat_constant>(colors::blue, 0.04, 0.0 );
        auto frosted = object_library::make_sphere_medium(point3( 190, 90, 190 ), 90, frosted_mat);
        mediums.add(frosted);

        surface_list surface_lights;
        surface_lights.add( light );

        medium_list medium_lights;

        auto const skybox = make_shared<solid_color_skybox>((colors::black));

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene cornell_ball() {
        camera cam;
        cam.vfov = 40;
        cam.lookfrom = point3( 278, 278, -800 );
        cam.lookat = point3( 278, 278, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        auto red = make_shared<lambertian>( colors::n_red );
        auto white = make_shared<lambertian>( colors::n_white );
        auto green = make_shared<lambertian>( colors::n_green );
        auto aluminum = make_shared<metal>( colors::metal_grey, 0.01 );
        surfaces.add( object_library::make_quad( point3( 555, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), green ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), red ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 555, 0, 0 ), vec3( 0, 0, 555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 555, 555, 555 ), vec3( -555, 0, 0 ), vec3( 0, 0, -555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 555 ), vec3( 555, 0, 0 ), vec3( 0, 555, 0 ), white ) );
        // Box
        shared_ptr<surface> box1 = object_library::make_box( point3( 0, 0, 0 ), point3( 165, 330, 165 ), white );
        box1 = object_library::make_rotate( box1, 15 );
        box1 = object_library::make_translate( box1, vec3( 265, 0, 295 ) );
        surfaces.add( box1 );
        // Glass Sphere
        auto glass_mat = make_shared<dielectric>( 1.5 );
        auto glass_ball = object_library::make_sphere( point3( 190, 90, 190 ), 90, glass_mat );
        surfaces.add( glass_ball );

        // Light Sources
        auto light_mat = make_shared<emissive>( colors::bright_light );

        auto light = object_library::make_quad( point3( 343, 554, 332 ), vec3( -130, 0, 0 ), vec3( 0, 0, -105 ), light_mat );
        // Light
        surfaces.add( light );

        medium_list mediums;

        surface_list surface_lights;
        surface_lights.add( light );

        medium_list medium_lights;

        auto const skybox = make_shared<solid_color_skybox>((colors::black));

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene cornell_smoke() {
        camera cam;
        cam.vfov = 40;
        cam.lookfrom = point3( 278, 278, -800 );
        cam.lookat = point3( 278, 278, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        auto red = make_shared<lambertian>( colors::n_red );
        auto white = make_shared<lambertian>( colors::n_white );
        auto green = make_shared<lambertian>( colors::n_green );

        surfaces.add( object_library::make_quad( point3( 555, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), green ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), red ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 555, 0, 0 ), vec3( 0, 0, 555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 555, 555, 555 ), vec3( -555, 0, 0 ), vec3( 0, 0, -555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 555 ), vec3( 555, 0, 0 ), vec3( 0, 555, 0 ), white ) );

        auto light_mat = make_shared<emissive>( colors::bright_light );
        auto light = object_library::make_quad( point3( 343, 554, 332 ), vec3( -130, 0, 0 ), vec3( 0, 0, -105 ), light_mat );
        surfaces.add( light );

        medium_list mediums;

        // Scattering-dominant smoke cloud in center of cornell box
        vec3 half_size(120, 180, 120);
        aabb smoke_bounds(-half_size, half_size);
        auto grid = make_smoke_grid(40, 40, 40, smoke_bounds);

        auto base_mat = make_shared<medium_mat_hg_constant>(
            color(0.002, 0.002, 0.002),  // sigma_a
            color(0.04, 0.04, 0.04),     // sigma_s (τ ≈ 2.9 through center)
            colors::black,               // emission
            0.4                          // g (moderate forward scatter for smoke)
        );
        auto grid_mat = make_shared<medium_mat_grid>(grid, base_mat);

        auto smoke = object_library::make_box_medium(point3(278, 200, 278), half_size, grid_mat);
        mediums.add(smoke);

        surface_list surface_lights;
        surface_lights.add( light );

        medium_list medium_lights;

        auto const skybox = make_shared<solid_color_skybox>(colors::black);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    // =========================================================================
    // Cloud in open sky with ground plane and directional light
    // =========================================================================
    static scene clouds() {
        camera cam;
        cam.vfov = 45;
        cam.lookfrom = point3( 0, 120, -800 );
        cam.lookat = point3( 0, 280, 100 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        medium_list mediums;

        // Ground plane
        auto ground_mat = make_shared<lambertian>(color(0.4, 0.5, 0.3));
        surfaces.add(object_library::make_quad(point3(-3000, 0, -3000), vec3(6000, 0, 0), vec3(0, 0, 6000), ground_mat));

        // Sun - high and behind/above the cloud for top-lit look
        auto light_mat = make_shared<emissive>( color(18, 16, 12) );
        auto light = object_library::make_quad( point3( -100, 600, 0 ), vec3( 200, 0, 0 ), vec3( 0, 0, 200 ), light_mat );
        surfaces.add( light );

        // Large cumulus cloud
        vec3 half_size(300, 200, 250);
        aabb bounds(-half_size, half_size);
        auto grid = make_cloud_grid(64, 48, 64, bounds);

        // sigma_s: path through center ≈ 600, avg density ≈ 0.25: τ ≈ 0.03 * 0.25 * 600 ≈ 4.5
        auto cloud_base = make_shared<medium_mat_hg_constant>(
            color(0.0001, 0.0001, 0.0001), // sigma_a (near-zero for white cloud)
            color(0.03, 0.03, 0.03),       // sigma_s
            colors::black,                  // emission
            0.76                            // g (strong forward scattering)
        );
        auto grid_mat = make_shared<medium_mat_grid>(grid, cloud_base);

        mediums.add(object_library::make_box_medium(point3(0, 320, 100), half_size, grid_mat));

        surface_list surface_lights;
        surface_lights.add( light );

        medium_list medium_lights;

        auto const skybox = make_shared<solid_color_skybox>(colors::black);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    // =========================================================================
    // Volumetric god-rays: spotlight through fog in a dark room
    // =========================================================================
    static scene god_rays() {
        camera cam;
        cam.vfov = 50;
        cam.lookfrom = point3( 278, 200, -500 );
        cam.lookat = point3( 278, 200, 278 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        auto dark_wall = make_shared<lambertian>( color(0.15, 0.15, 0.18) );
        auto floor_mat = make_shared<lambertian>( color(0.3, 0.25, 0.2) );

        // Room walls
        surfaces.add( object_library::make_quad( point3( 555, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), dark_wall ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), dark_wall ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 555, 0, 0 ), vec3( 0, 0, 555 ), floor_mat ) );
        surfaces.add( object_library::make_quad( point3( 555, 555, 555 ), vec3( -555, 0, 0 ), vec3( 0, 0, -555 ), dark_wall ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 555 ), vec3( 555, 0, 0 ), vec3( 0, 555, 0 ), dark_wall ) );

        // Small bright ceiling light (creates shaft of light)
        auto light_mat = make_shared<emissive>( color(30, 28, 22) );
        auto light = object_library::make_quad( point3( 248, 554, 248 ), vec3( 60, 0, 0 ), vec3( 0, 0, 60 ), light_mat );
        surfaces.add( light );

        // Reflective sphere on the floor to catch god rays
        auto mirror_mat = make_shared<metal>( color(0.9, 0.9, 0.95), 0.02 );
        surfaces.add( object_library::make_sphere( point3(278, 80, 278), 80, mirror_mat ) );

        // Homogeneous participating medium filling the room (light fog)
        medium_list mediums;
        auto fog_mat = make_shared<medium_mat_hg_constant>(
            color(0.0003, 0.0003, 0.0003),  // sigma_a
            color(0.003, 0.003, 0.003),     // sigma_s (τ ≈ 1.8 across room)
            colors::black,                   // no emission
            0.5                              // g (moderate forward scatter for god rays)
        );
        vec3 room_half(277, 277, 277);
        mediums.add(object_library::make_box_medium(point3(278, 278, 278), room_half, fog_mat));

        surface_list surface_lights;
        surface_lights.add( light );
        medium_list medium_lights;

        auto const skybox = make_shared<solid_color_skybox>(colors::black);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    // =========================================================================
    // Nebula: coloured overlapping volumetric clouds in space
    // =========================================================================
    static scene nebula() {
        camera cam;
        cam.vfov = 45;
        cam.lookfrom = point3( 0, 0, -500 );
        cam.lookat = point3( 0, 0, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        medium_list mediums;

        // Embedded star light sources
        auto star1_mat = make_shared<emissive>( color(12, 10, 6) );
        auto star1 = object_library::make_sphere( point3(-60, 20, 50), 15, star1_mat );
        surfaces.add( star1 );

        auto star2_mat = make_shared<emissive>( color(4, 4, 12) );
        auto star2 = object_library::make_sphere( point3(80, -30, -20), 10, star2_mat );
        surfaces.add( star2 );

        // Red/orange nebula cloud
        vec3 hs1(120, 90, 120);
        double r1 = std::min({hs1.x(), hs1.y(), hs1.z()});
        aabb bounds1(-hs1, hs1);
        auto grid1 = make_smoke_grid(25, 20, 25, bounds1);
        // path ≈ 240, avg density ≈ 0.19: τ_r = 0.025×0.19×240 ≈ 1.1
        auto red_cloud_mat = make_shared<medium_mat_constant>(
            color(0.001, 0.004, 0.004),    // sigma_a: absorb green/blue
            color(0.025, 0.006, 0.003),    // sigma_s: scatter red
            colors::black
        );
        auto grid_mat1 = make_shared<medium_mat_grid>(grid1, red_cloud_mat);
        mediums.add(object_library::make_sphere_medium(point3(-40, 10, 30), r1, grid_mat1));

        // Blue/purple nebula cloud (overlapping)
        vec3 hs2(100, 80, 100);
        double r2 = std::min({hs2.x(), hs2.y(), hs2.z()});
        aabb bounds2(-hs2, hs2);
        auto grid2 = make_smoke_grid(22, 18, 22, bounds2);
        // path ≈ 200, avg density ≈ 0.19: τ_b = 0.025×0.19×200 ≈ 1.0
        auto blue_cloud_mat = make_shared<medium_mat_constant>(
            color(0.004, 0.002, 0.0005),   // sigma_a: absorb red
            color(0.004, 0.008, 0.025),    // sigma_s: scatter blue
            colors::black
        );
        auto grid_mat2 = make_shared<medium_mat_grid>(grid2, blue_cloud_mat);
        mediums.add(object_library::make_sphere_medium(point3(50, -20, -10), r2, grid_mat2));

        surface_list surface_lights;
        surface_lights.add( star1 );
        surface_lights.add( star2 );
        medium_list medium_lights;

        auto const skybox = make_shared<solid_color_skybox>(color(0.002, 0.002, 0.005));

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    // =========================================================================
    // Glass sphere with interior fog + exterior atmosphere
    // =========================================================================
    static scene foggy_glass() {
        camera cam;
        cam.vfov = 40;
        cam.lookfrom = point3( 278, 278, -800 );
        cam.lookat = point3( 278, 278, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        auto red = make_shared<lambertian>( colors::n_red );
        auto white = make_shared<lambertian>( colors::n_white );
        auto green = make_shared<lambertian>( colors::n_green );

        // Cornell box walls
        surfaces.add( object_library::make_quad( point3( 555, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), green ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 0, 555, 0 ), vec3( 0, 0, 555 ), red ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 0 ), vec3( 555, 0, 0 ), vec3( 0, 0, 555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 555, 555, 555 ), vec3( -555, 0, 0 ), vec3( 0, 0, -555 ), white ) );
        surfaces.add( object_library::make_quad( point3( 0, 0, 555 ), vec3( 555, 0, 0 ), vec3( 0, 555, 0 ), white ) );

        // Box pedestal
        shared_ptr<surface> pedestal = object_library::make_box( point3( 0, 0, 0 ), point3( 165, 100, 165 ), white );
        pedestal = object_library::make_rotate( pedestal, 15 );
        pedestal = object_library::make_translate( pedestal, vec3( 265, 0, 295 ) );
        surfaces.add( pedestal );

        // Glass sphere on pedestal
        auto glass_mat = make_shared<dielectric>( 1.5 );
        surfaces.add( object_library::make_sphere( point3(340, 220, 370), 100, glass_mat ) );

        // Light
        auto light_mat = make_shared<emissive>( colors::bright_light );
        auto light = object_library::make_quad( point3( 343, 554, 332 ), vec3( -130, 0, 0 ), vec3( 0, 0, -105 ), light_mat );
        surfaces.add( light );

        medium_list mediums;

        // Coloured fog inside the glass sphere
        auto inner_fog = make_shared<medium_mat_constant>(
            color(0.004, 0.001, 0.001),   // sigma_a
            color(0.012, 0.003, 0.003),   // sigma_s (reddish, τ ≈ 3 across diameter)
            colors::black
        );
        mediums.add(object_library::make_sphere_medium(point3(340, 220, 370), 99, inner_fog));

        // Light atmospheric haze in the room
        auto room_fog = make_shared<medium_mat_constant>(
            color(0.0002, 0.0002, 0.0002),
            color(0.001, 0.001, 0.001),    // (τ ≈ 0.7 across room)
            colors::black
        );
        vec3 room_half(277, 277, 277);
        mediums.add(object_library::make_box_medium(point3(278, 278, 278), room_half, room_fog));

        surface_list surface_lights;
        surface_lights.add( light );
        medium_list medium_lights;

        auto const skybox = make_shared<solid_color_skybox>(colors::black);

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

    static scene sunset_clouds() {
        camera cam;
        cam.vfov = 50;
        cam.lookfrom = point3( 0, 150, 0 );
        cam.lookat   = point3( 0, 280, 600 );
        cam.vup      = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        medium_list  mediums;

        // Ground (dark earth tones)
        auto ground_mat = make_shared<lambertian>(color(0.15, 0.12, 0.08));
        surfaces.add(object_library::make_quad(
            point3(-3000, 0, -3000), vec3(6000, 0, 0), vec3(0, 0, 6000), ground_mat));

        // Low-angle warm "sun" light (sunset from the side, much brighter)
        auto sun_mat = make_shared<emissive>( color(60, 25, 6) );
        auto sun = object_library::make_quad(
            point3( 1500, 180, 800 ), vec3( 0, 200, 0 ), vec3( 0, 0, 200 ), sun_mat );
        surfaces.add( sun );

        // Shared cloud material — scattering raised substantially so clouds are visible
        auto cloud_mat1 = make_shared<medium_mat_hg_constant>(
            color(0.02, 0.02, 0.02),   // absorption
            color(0.5,  0.5,  0.5 ),   // scattering (was 0.018 — far too low)
            colors::black,
            0.76                        // strong forward scatter
        );

        // Cloud layer 1 - wide flat cloud
        vec3 hs1(250, 40, 200);
        aabb b1(-hs1, hs1);
        auto g1  = make_cloud_grid(28, 12, 28, b1);
        auto gm1 = make_shared<medium_mat_grid>(g1, cloud_mat1);
        mediums.add(object_library::make_box_medium(point3(-50, 300, 500), hs1, gm1));

        // Cloud layer 2 - smaller high cloud
        vec3 hs2(120, 35, 100);
        aabb b2(-hs2, hs2);
        auto g2  = make_cloud_grid(20, 10, 20, b2);
        auto gm2 = make_shared<medium_mat_grid>(g2, cloud_mat1);
        mediums.add(object_library::make_box_medium(point3(200, 380, 700), hs2, gm2));

        // Cloud layer 3 - small wispy cloud
        vec3 hs3(80, 25, 70);
        aabb b3(-hs3, hs3);
        auto g3  = make_cloud_grid(16, 8, 16, b3);
        auto gm3 = make_shared<medium_mat_grid>(g3, cloud_mat1);
        mediums.add(object_library::make_box_medium(point3(-200, 340, 400), hs3, gm3));

        surface_list surface_lights;
        surface_lights.add( sun );

        medium_list medium_lights;

        // Sunset gradient: warm orange at horizon, deep purple/blue above
        auto const skybox = make_shared<gradient_skybox>(
            color(0.7, 0.3, 0.05),   // horizon — warm orange
            color(0.08, 0.05, 0.18)  // zenith  — deep blue-purple
        );

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

private:
    // Smoke/turbulence grid with sharp falloff
    static shared_ptr<density_grid> make_smoke_grid(int nx, int ny, int nz, const aabb& bounds) {
        auto grid = make_shared<density_grid>();
        grid->nx = nx;
        grid->ny = ny;
        grid->nz = nz;
        grid->bounds = bounds;
        grid->data.resize(nx * ny * nz);

        perlin p;
        for (int k = 0; k < nz; k++) {
            for (int j = 0; j < ny; j++) {
                for (int i = 0; i < nx; i++) {
                    double x = (double)i / (nx - 1);
                    double y = (double)j / (ny - 1);
                    double z = (double)k / (nz - 1);

                    point3 pos(x * 4.0, y * 4.0, z * 4.0);
                    double density = std::abs(p.turb(pos, 7));

                    // Remap: threshold low values to create empty space between wisps
                    double threshold = 0.35;
                    density = std::max(0.0, (density - threshold) / (1.0 - threshold));

                    // Spherical falloff from center to avoid visible box boundaries
                    double cx = 2.0 * x - 1.0;  // map [0,1] -> [-1,1]
                    double cy = 2.0 * y - 1.0;
                    double cz = 2.0 * z - 1.0;
                    double r2 = cx*cx + cy*cy + cz*cz;
                    double falloff = std::max(0.0, 1.0 - r2);
                    falloff *= falloff;  // quartic spherical falloff
                    density *= falloff;

                    grid->data[i + j * nx + k * nx * ny] = (float)density;
                }
            }
        }
        grid->precompute();
        return grid;
    }

    // Cloud grid: smoother, wider shapes with softer edges
    // Smooth metaball-style falloff: 1 at center, 0 at distance >= radius
    static double lobe(double dx, double dy, double dz, double cx, double cy, double cz, double r) {
        double ex = (dx - cx) / r;
        double ey = (dy - cy) / r;
        double ez = (dz - cz) / r;
        double d2 = ex*ex + ey*ey + ez*ez;
        if (d2 >= 1.0) return 0.0;
        double t = 1.0 - d2;
        return t * t; // smooth quartic falloff
    }

    static shared_ptr<density_grid> make_cloud_grid(int nx, int ny, int nz, const aabb& bounds) {
        auto grid = make_shared<density_grid>();
        grid->nx = nx;
        grid->ny = ny;
        grid->nz = nz;
        grid->bounds = bounds;
        grid->data.resize(nx * ny * nz);

        perlin p;

        // Multi-lobe cumulus structure:
        // coords in [-1, 1] normalized space
        // Lobes: (cx, cy, cz, radius, weight)
        // Tall dome on top, wider base, secondary turrets
        struct Lobe { double cx, cy, cz, r, w; };
        Lobe lobes[] = {
            // Main central dome (large, top-heavy)
            {  0.0,   0.15,  0.0,   0.65,  1.0  },
            // Upper turrets (billowy top)
            { -0.25,  0.45,  0.0,   0.40,  0.9  },
            {  0.25,  0.50, -0.10,  0.35,  0.85 },
            {  0.0,   0.55,  0.15,  0.30,  0.8  },
            // Side lobes (width)
            { -0.50,  0.0,   0.10,  0.40,  0.7  },
            {  0.50,  0.0,  -0.05,  0.38,  0.7  },
            {  0.0,  -0.05,  0.40,  0.35,  0.6  },
            {  0.0,  -0.05, -0.40,  0.35,  0.6  },
            // Smaller detail lobes
            { -0.15,  0.60,  0.20,  0.22,  0.5  },
            {  0.30,  0.35,  0.25,  0.28,  0.55 },
            { -0.35,  0.30, -0.20,  0.25,  0.5  },
        };
        int n_lobes = sizeof(lobes) / sizeof(lobes[0]);

        for (int k = 0; k < nz; k++) {
            for (int j = 0; j < ny; j++) {
                for (int i = 0; i < nx; i++) {
                    // Normalized [0,1]
                    double u = (double)i / (nx - 1);
                    double v = (double)j / (ny - 1);
                    double w = (double)k / (nz - 1);

                    // Map to [-1, 1]
                    double dx = 2.0 * u - 1.0;
                    double dy = 2.0 * v - 1.0;
                    double dz = 2.0 * w - 1.0;

                    // Sum all lobes (soft union via max-like blend)
                    double shape = 0.0;
                    for (int l = 0; l < n_lobes; l++) {
                        shape = std::max(shape, lobes[l].w * lobe(dx, dy, dz,
                            lobes[l].cx, lobes[l].cy, lobes[l].cz, lobes[l].r));
                    }

                    if (shape < 0.01) {
                        grid->data[i + j * nx + k * nx * ny] = 0.0f;
                        continue;
                    }

                    // Multi-octave noise for billowy detail
                    point3 pos(u * 6.0, v * 6.0, w * 6.0);
                    double noise_val = 0.5 + 0.5 * p.turb(pos, 6);

                    // Finer detail layer
                    point3 pos2(u * 14.0 + 3.7, v * 14.0 + 1.2, w * 14.0 + 5.9);
                    double detail = 0.5 + 0.5 * p.turb(pos2, 4);

                    // Blend: shape modulated by noise
                    double density = shape * (0.6 * noise_val + 0.4 * detail);

                    // Vertical gradient: denser at top, wispier at bottom
                    double vert = (dy + 1.0) * 0.5; // 0 at bottom, 1 at top
                    double vert_mod = 0.4 + 0.6 * vert;
                    density *= vert_mod;

                    grid->data[i + j * nx + k * nx * ny] = (float)std::max(0.0, density);
                }
            }
        }
        grid->precompute();
        return grid;
    }
};

#endif //SCENE_LIBRARY_H
