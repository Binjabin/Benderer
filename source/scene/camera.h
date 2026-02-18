//
// Created by binjabin on 6/24/25.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "world.h"
#include "hittables/hittable.h"
#include "material/surface_material.h"
#include "../structures/pdf.h"
#include "../image/image_info.h"
#include "../integrators/integrator.h"
#include "../image/writer/image_writer.h"
#include "../image/post/post_process.h"


class camera {
public:
    double vfov = 90; // Vertical view angle fov
    point3 lookfrom = point3( 0, 0, 0 );
    point3 lookat = point3( 0, 0, -1 );
    vec3 vup = vec3( 0, 1, 0 );

    double defocus_angle = 0; //variation of angle of rays through each pixel
    double focus_dist = 10; //distance from camera look point to distance of perfect focus

    void render(const std::string& filename, const world& world, image_info info, const integrator& itgr, post_process& post, const image_writer& preview_writer, const image_writer& result_writer) {
        initialize(info);

        int ph = info.pixel_height();
        int pw = info.pixel_width();
        int spp = info.samples_per_pixel();
        int md = info.max_depth();
        double ss = info.sample_scale();

        size_t pixel_area = size_t(pw) * size_t(ph);

        std::vector<double> display_buff;
        display_buff.resize( pixel_area * 3 );
        std::vector<double> accum_buff;
        accum_buff.resize( pixel_area * 3 );

        const int preview_update_frequency = 40000;
        int work_until_update = 0;

        for ( int sample = 0; sample < spp; sample++ ) {

            std::clog << "\rSamples: " << ( sample ) << ' ' << std::flush;

            for ( int j = 0; j < ph ; j++ ) {

                for ( int i = 0; i < pw ; i++ ) {
                    color pixel_color( 0, 0, 0 );
                    ray r = get_ray( i, j );
                    pixel_color += itgr.ray_color( r, md, world );

                    const size_t idx = (size_t(j) * size_t(pw) + size_t(i)) * 3;

                    accum_buff[idx + 0] += pixel_color.x(); //R
                    accum_buff[idx + 1] += pixel_color.y(); //G
                    accum_buff[idx + 2] += pixel_color.z(); //B

                    work_until_update++;
                    if (work_until_update >= preview_update_frequency) {
                        work_until_update = 0;
                        update_preview(display_buff, accum_buff, post, preview_writer, pixel_area, (sample+1));
                    }
                }
            }
        }

        //Reset preview:
        std::vector<double> black(ph * pw * 3, 0.0);
        preview_writer.write("preview", black);

        accum_to_display(display_buff, accum_buff, post, pixel_area, ss);
        result_writer.write(filename, display_buff);
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

    void update_preview(std::vector<double>& display_buff, const std::vector<double>& accum_buff, post_process& post, const image_writer& preview_writer, size_t size, int samples) {
        // compute averaged display buffer (do not modify accum)
        double ss = (1.0 / double(samples));

        accum_to_display(display_buff, accum_buff, post, size, ss);

        preview_writer.write("preview", display_buff);
    }

    void accum_to_display(std::vector<double>& display_buff, const std::vector<double>& accum_buff, post_process& post, size_t size, double fac) {
        for (size_t p = 0; p < size; ++p) {
            size_t base = p * 3;
            display_buff[base + 0] = accum_buff[base + 0] * fac;
            display_buff[base + 1] = accum_buff[base + 1] * fac;
            display_buff[base + 2] = accum_buff[base + 2] * fac;
        }

        post.apply_post(display_buff);
    }
};

#endif //CAMERA_H
