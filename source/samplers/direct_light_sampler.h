//
// Created by binjabin on 2/19/26.
//

#ifndef BENDERER_DIRECT_LIGHT_SAMPLER_H
#define BENDERER_DIRECT_LIGHT_SAMPLER_H
#include <memory>


#include "../scene/world.h"
#include "../scene/material/surface_material.h"
#include "../scene/hittables/surfaces/surface.h"
#include "../scene/hittables/hittable.h"

class direct_light_sampler {
public:

    //For when we have a point to sample from
    static bool sample_from_point(const world& world, const point3& from, local_light_sample& light_sample) {

        const auto sky = world.m_sky;
        const auto lights = world.m_lights;

        //Values for MIS/pdfs
        const double sky_weight = sky ? sky->get_flux_weight() : 0.0;
        const bool have_lights = (lights && lights->get_count() && lights->get_flux_weight() > 0.0);
        const double lights_weight = have_lights ? (std::max(lights->get_flux_weight(), 0.0)) : 0.0;
        const double flux_sum = sky_weight + lights_weight;

        if (flux_sum <= 0.0) return false;

        const double sample_seed = flux_sum * random_double();

        if (sample_seed < sky_weight || lights_weight <= 0.0) {
            //Sample from skybox
            double p = sky_weight / flux_sum;

            const environment_light_sample env_sample = sky->sample_light_over_flux(p);
            light_sample = env_sample.to_local_sample(from);
            return true;
        }

        const double new_seed = (sample_seed - sky_weight) / std::max(epsilon, lights_weight);
        const double p = lights_weight / flux_sum;

        const surface_light_sample ls = lights->sample_light_over_flux(new_seed, p);
        light_sample = ls.to_local_sample(from);

        return true;
    }

    //For when we just want to sample a light ray
    static bool sample_from_scene(const world& world, light_ray_sample& light_sample) {

        light_sample = light_ray_sample();

        const auto sky = world.m_sky;
        const auto lights = world.m_lights;

        //Values for MIS/pdfs
        const double sky_weight = sky ? sky->get_flux_weight() : 0.0;
        const bool have_lights = (lights && lights->get_count() && lights->get_flux_weight() > 0.0);
        const double lights_weight = have_lights ? (std::max(lights->get_flux_weight(), 0.0)) : 0.0;
        const double flux_sum = sky_weight + lights_weight;

        if (flux_sum <= 0.0) return false;

        const double sample_seed = flux_sum * random_double();

        if (sample_seed < sky_weight || lights_weight <= 0.0) {
            //Sample from skybox
            double p = sky_weight / flux_sum;

            //Get direction etc
            const environment_light_sample env_sample = sky->sample_light_over_flux(p);
            //TODO: Create disc and sample around that.

            vec3 light_dir = env_sample.m_direction;

            vec3 disk_sample = random_in_unit_disk() * world.m_furthest_distance;
            onb disk_onb = onb(light_dir);
            vec3 offset = disk_onb.transform(disk_sample);
            double disk_pdf = 1.0 / (pi * world.m_furthest_distance * world.m_furthest_distance);
            point3 ray_origin = -(light_dir * (world.m_furthest_distance + epsilon)) + offset;
            
            light_sample.m_radiance = env_sample.m_radiance;
            light_sample.m_ray = ray(ray_origin, light_dir);
            light_sample.m_pdf_w = p * env_sample.m_pdf_w * disk_pdf;
            light_sample.m_is_env_light = true;

            return true;
        }

        const double new_seed = (sample_seed - sky_weight) / std::max(epsilon, lights_weight);
        const double p = lights_weight / flux_sum;

        const surface_light_sample ls = lights->sample_light_over_flux(new_seed, p);

        //Get direction out of surface sample
        cosine_pdf cos_pdf(ls.m_normal);
        pdf_rec rec;
        cos_pdf.sample(rec);

        light_sample.m_radiance = ls.m_radiance;
        light_sample.m_ray = ray(ls.m_light_p, rec.direction);
        light_sample.m_pdf_w = p * ls.m_pdf_A * rec.pdf;
        light_sample.m_is_env_light = false;

        return true;
    }

    static double pdf_w(const world& world, const point3& from, const vec3& direction) {
        const auto sky = world.m_sky;
        const auto lights = world.m_lights;

        //Values for MIS/pdfs
        const double sky_weight = sky ? sky->get_flux_weight() : 0.0;
        const bool have_lights = (lights && lights->get_count() && lights->get_flux_weight() > 0.0);
        const double lights_weight = have_lights ? (std::max(lights->get_flux_weight(), 0.0)) : 0.0;
        const double flux_sum = sky_weight + lights_weight;

        if (flux_sum <= 0.0) return 0.0;

        const vec3 w = unit_vector(direction);

        double pdf = 0.0;

        if (sky_weight > 0.0 && sky) {
            const double p_sky = sky_weight / flux_sum;
            pdf += p_sky * sky->get_pdf_value(w);
        }

        if (lights_weight > 0.0 && lights) {
            surface_hit_rec light_rec;
            ray light_ray = ray(from, w);
            if (lights->surface_hit(light_ray, interval(epsilon, infinity), light_rec)) {
                const vec3 to_light = light_rec.get_p() - from;
                const double dist2 = to_light.length_squared();
                if (dist2 > epsilon) {
                    const double cos_light = dot(light_rec.get_normal(), -w);
                    if (cos_light > epsilon) {
                        const double p_light = lights_weight / flux_sum;
                        const double pdf_a = light_rec.m_pdf_v;
                        pdf += p_light * pdf_a * (dist2 / cos_light);
                    }
                }
            }
        }

        return pdf;
    }


private:
};

#endif //BENDERER_DIRECT_LIGHT_SAMPLER_H