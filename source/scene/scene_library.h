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

        // Define scattering-dominant smoke cloud in center
        vec3 half_size(120, 180, 120);
        aabb smoke_bounds(-half_size, half_size);

        auto grid = make_smoke_grid(40, 40, 40, smoke_bounds);

        auto base_mat = make_shared<medium_mat_constant>(
            color(0.05, 0.05, 0.05),  // sigma_a (absorption)
            color(0.25, 0.25, 0.25),  // sigma_s (scattering)
            colors::black             // emission
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

    static scene clouds() {
        camera cam;
        cam.vfov = 35;
        cam.lookfrom = point3( 0, 300, -800 );
        cam.lookat = point3( 0, 300, 0 );
        cam.vup = vec3( 0, 1, 0 );
        cam.defocus_angle = 0;

        surface_list surfaces;
        medium_list mediums;

        // Ground plane to show shadow
        auto ground_mat = make_shared<lambertian>(colors::n_white);
        surfaces.add(object_library::make_quad(point3(-2000, 0, -2000), vec3(4000, 0, 0), vec3(0, 0, 4000), ground_mat));

        // Small, bright directional light to cast sharp shadows
        auto light_mat = make_shared<emissive>( colors::bright_light );
        auto light = object_library::make_quad( point3( -400, 1000, -400 ), vec3( 800, 0, 0 ), vec3( 0, 0, 800 ), light_mat );
        surfaces.add( light );

        // Single grey cloud centered in view
        vec3 half_size(200, 80, 200);
        aabb bounds(-half_size, half_size);
        auto grid = make_smoke_grid(25, 20, 25, bounds);

        auto cloud_base = make_shared<medium_mat_constant>(color(0.5, 0.5, 0.5), 0.12, 0.11);
        auto grid_mat = make_shared<medium_mat_grid>(grid, cloud_base);

        mediums.add(object_library::make_box_medium(point3(0, 300, 200), half_size, grid_mat));

        surface_list surface_lights;
        surface_lights.add( light );

        medium_list medium_lights;

        auto const skybox = make_shared<gradient_skybox>(color(0.3, 0.3, 0.3), color(0.05, 0.05, 0.1));

        return scene( cam,
            make_shared<surface_list>(surfaces),
            make_shared<medium_list>(mediums),
            make_shared<surface_list>(surface_lights),
            make_shared<medium_list>(medium_lights),
            skybox
        );
    }

private:
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

                    // Simple falloff towards edges
                    density *= std::sin(x * pi) * std::sin(y * pi) * std::sin(z * pi);

                    grid->data[i + j * nx + k * nx * ny] = (float)density;
                }
            }
        }
        grid->precompute();
        return grid;
    }
};

#endif //SCENE_LIBRARY_H
