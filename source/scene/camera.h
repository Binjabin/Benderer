//
// Created by binjabin on 6/24/25.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "hittables/hittable.h"
#include "material/material.h"
#include "../structures/pdf.h"
#include "../image/image_info.h"
#include "../integrators/integrator.h"

class camera {
public:
    double vfov = 90; // Vertical view angle fov
    point3 lookfrom = point3( 0, 0, 0 );
    point3 lookat = point3( 0, 0, -1 );
    vec3 vup = vec3( 0, 1, 0 );

    double defocus_angle = 0; //variation of angle of rays through each pixel
    double focus_dist = 10; //distance from camera look point to distance of perfect focus

    void render( const hittable& world, const hittable& lights, const shared_ptr<skybox> skybox, image_info info, const integrator& itgr) {
        initialize(info);

        std::cout << "P3\n" << info.pixel_width() << " " << info.pixel_height() << "\n255\n";

        int ph = info.pixel_height();
        int pw = info.pixel_width();
        int spp = info.samples_per_pixel();
        int md = info.max_depth();
        double ss = info.sample_scale();

        for ( int j = 0; j < ph ; j++ ) {
            std::clog << "\rScanlines remaining: " << ( ph - j ) << ' ' << std::flush;

            for ( int i = 0; i < pw ; i++ ) {
                color pixel_color( 0, 0, 0 );
                for ( int sample = 0; sample < spp; sample++ ) {
                    ray r = get_ray( i, j );
                    pixel_color += itgr.ray_color( r, md, world, lights, skybox );
                }

                color average = ss * pixel_color;
                double length = average.length();
                if (average.length() > 0.3) {
                    std::clog << "COLOR: " << average.length() << std::endl;
                }
                if (average.length() > 1) {
                    std::clog << "BIG COLOR" << std::endl;
                }

                write_color( std::cout, average );
            }
        }

        std::clog << "\rDone. \n";
    }

private:
    point3 center;
    point3 pixel00_loc;
    vec3 pixel_delta_u, pixel_delta_v;
    vec3 u, v, w; //camera frame basis
    vec3 defocus_disk_u, defocus_disk_v;

    void initialize(image_info info) {
        //IMAGE
        center = lookfrom;

        //VIEWPORT
        auto theta = degrees_to_radians( vfov );
        auto h = std::tan( theta / 2 );
        auto viewport_height = 2 * h * focus_dist;
        //aspect ratio is just a target. actual ratio may be different
        auto viewport_width = viewport_height * info.actual_ratio();

        w = unit_vector( lookfrom - lookat );
        u = unit_vector( cross( vup, w ) );
        v = cross( w, u );

        //Vectors along viewport edges
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;

        //pixel to pixel delta vectors
        pixel_delta_u = viewport_u / info.pixel_width();
        pixel_delta_v = viewport_v / info.pixel_height();

        //UPPER LEFT LOCATION
        auto viewport_upper_left = center - ( focus_dist * w ) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * ( pixel_delta_u + pixel_delta_v );

        //defocus disk basis
        auto defocus_radius = focus_dist * std::tan( degrees_to_radians( defocus_angle / 2 ) );
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray( int i, int j ) const {
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
            + ( ( i + offset.x() ) * pixel_delta_u )
            + ( ( j + offset.y() ) * pixel_delta_v );

        auto ray_origin = ( defocus_angle <= 0 ) ? center : defocus_disk_sample();

        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return ray( ray_origin, ray_direction, ray_time );
    }

    vec3 sample_square() const {
        auto rand1 = random_double();
        auto rand2 = random_double();
        return vec3( rand1 - 0.5, rand2 - 0.5, 0.0 );
    }

    vec3 defocus_disk_sample() const {
        auto p = random_in_unit_disk();
        return center + ( p[0] * defocus_disk_u ) + ( p[1] * defocus_disk_v );
    }

};

#endif //CAMERA_H
