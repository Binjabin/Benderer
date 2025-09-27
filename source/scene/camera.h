//
// Created by binjabin on 6/24/25.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "hittables/hittable.h"
#include "material/material.h"
#include "../structures/pdf.h"

class camera {
public:
    double aspect_ratio = 1.0; // Ratio of image width over height
    int image_width = 100; // Rendered image width in pixel count
    int samples_per_pixel = 10; // Count of random samples of each pixel
    int max_depth = 10; // Maximum number of ray bounces into a scene
    color background; //scene background color

    double vfov = 90; // Vertical view angle fov
    point3 lookfrom = point3( 0, 0, 0 );
    point3 lookat = point3( 0, 0, -1 );
    vec3 vup = vec3( 0, 1, 0 );

    double defocus_angle = 0; //variation of angle of rays through erach pixel
    double focus_dist = 10; //distance from camera look point to distance of perfect focus

    void render( const hittable& world, const hittable& lights ) {
        initialize();

        std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

        for ( int j = 0; j < image_height; j++ ) {
            std::clog << "\rScanlines remaining: " << ( image_height - j ) << ' ' << std::flush;
            for ( int i = 0; i < image_width; i++ ) {
                color pixel_color( 0, 0, 0 );
                for ( int sample = 0; sample < samples_per_pixel; sample++ ) {
                    ray r = get_ray( i, j );
                    pixel_color += ray_color( r, max_depth, world, lights );
                }

                write_color( std::cout, pixel_samples_scale * pixel_color );
            }
        }

        std::clog << "\rDone. \n";
    }

private:
    int image_height;
    double pixel_samples_scale;
    point3 center;
    point3 pixel00_loc;
    vec3 pixel_delta_u, pixel_delta_v;
    vec3 u, v, w; //camera frame basis
    vec3 defocus_disk_u, defocus_disk_v;

    void initialize() {
        //IMAGE
        image_height = int( image_width / aspect_ratio );
        image_height = ( image_height < 1 ) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        //VIEWPORT
        auto theta = degrees_to_radians( vfov );
        auto h = std::tan( theta / 2 );
        auto viewport_height = 2 * h * focus_dist;
        //aspect ratio is just a target. actual ratio may be different
        auto viewport_width = viewport_height * ( double( image_width ) / image_height );

        w = unit_vector( lookfrom - lookat );
        u = unit_vector( cross( vup, w ) );
        v = cross( w, u );

        //Vectors along viewport edges
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;

        //pixel to pixel delta vectors
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

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

    color ray_color( const ray& r, int depth, const hittable& world, const hittable& lights ) const {
        //no more light is gathered
        if ( depth <= 0 ) {
            return color( 0, 0, 0 );
        }

        hit_record rec;
        auto ray_interval = interval( 0.001, infinity );

        if ( !world.hit( r, ray_interval, rec ) ) {
            return background;
        }


        scatter_record srec;
        color color_from_emission = rec.mat->emitted( r, rec, rec.u, rec.v, rec.p );

        //cast ray, get results
        //After this, pdf_value is the pdf of the actually generated ray through our SAMPLING SYSTEM.
        //ie how we chose the ray
        //we use this to weight the contribution later
        if ( !rec.mat->scatter( r, rec, srec ) ) {
            return color_from_emission;
        }

        if ( srec.skip_pdf ) {
            return srec.attenuation * ray_color( srec.skip_pdf_ray, depth - 1, world, lights );
        }

        auto light_ptr = make_shared<hittable_pdf>( lights, rec.p );
        mixture_pdf p( light_ptr, srec.pdf_ptr );

        ray scattered = ray( rec.p, p.generate(), r.time() );
        auto pdf_value = p.value( scattered.direction() );

        double scattering_pdf = rec.mat->scattering_pdf( r, rec, scattered );

        color sample_color = ray_color( scattered, depth - 1, world, lights );
        color color_from_scatter = ( srec.attenuation * scattering_pdf * sample_color ) / pdf_value;

        return color_from_emission + color_from_scatter;

        //SKYBOX
        //vec3 unit_direction = unit_vector( r.direction() );
        //auto a = 0.5 * ( unit_direction.y() + 1.0 );
        //return ( 1.0 - a ) * color( 1.0, 1.0, 1.0 ) + a * color( 0.5, 0.5, 0.7 );
    }
};

#endif //CAMERA_H
