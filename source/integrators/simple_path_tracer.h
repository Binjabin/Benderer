//
// Created by binjabin on 1/17/26.
//

#ifndef BENDERER_SIMPLE_PATH_TRACER_H
#define BENDERER_SIMPLE_PATH_TRACER_H
#include "integrator.h"
#include "../scene/material/material.h"
#include "../structures/path_result.h"
#include "../structures/path_state.h"

class simple_path_tracer : public integrator {

public:
    simple_path_tracer(int max_depth)
        : m_max_depth(max_depth) {
    };

    color ray_color(const ray &r, int depth, const hittable &world, const hittable &lights, const shared_ptr<skybox> sky) const override {
        path_state p_state = initial_path_state();
        path_result res = path_trace(r, world, lights, sky, p_state);
        return res.radiance_from_path;
    }


private:
    int m_max_depth;

    path_result path_trace(const ray& r, const hittable& world, const hittable& lights, const shared_ptr<skybox> sky, path_state& p_state) const {

        path_result out_result = path_result();

        //---------------------------------------
        // Terminate if our throughput becomes 0 (or close)
        if (max_component(p_state.overall_throughput) < epsilon) {
            return empty_path_result();
        }

        //---------------------------------------
        // First check that our ray hits anything
        hit_record rec;
        interval ray_t = interval(epsilon, infinity);
        //If it doesn't, default to skybox
        if ( !world.hit( r, ray_t, rec ) ) {
            color col_from_sky = sky->sample_color(r.direction());
            return color_path_result(col_from_sky);
        }

        //---------------------------------------
        // Get surface emittance
        color color_from_light = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

        //---------------------------------------
        // We have a max depth. Terminate if the next sample would exceed that depth.
        if (p_state.depth + 1 >= m_max_depth) {
            return color_path_result(color_from_light);
        }

        //---------------------------------------
        // Otherwise, calculate ray "scatter". Either absorb ray (just return emittance, no more bounces) or calculate the scatter direction

        scatter_record srec;
        bool scattered = rec.mat->scatter(r, rec, srec);

        //If we don't scatter, then we exit early
        if (!scattered) {
            path_result no_scatter = color_path_result(color_from_light);
            return no_scatter;
        }

        //----------------------------------------
        // If our ray is scattered, we take another step, an indirect sample
        // Generate a scatter direction, and calculate lighting from that direction down our ray

        path_result indirect_res = empty_path_result();
        path_state child_state = p_state;
        child_state.depth++;
        if (srec.skip_pdf) {
            //Don't use a pdf, just take the specified ray
            ray scattered_ray = srec.skip_pdf_ray;

            //prev bsdf is meaningless for specular. Just keep sensible.
            child_state.prev_bsdf_pdf = 1.0;

            //attenuation is like the throughput of spec surfaces
            child_state.overall_throughput = srec.attenuation * child_state.overall_throughput;
            indirect_res = path_trace(scattered_ray, world, lights, sky, child_state);
            indirect_res.radiance_from_path = indirect_res.radiance_from_path * srec.attenuation;
        }
        else {
            vec3 scatter_dir = srec.pdf_ptr->generate();
            ray scattered_ray = ray(rec.p, scatter_dir, r.time());

            double pdf_mat = rec.mat->scattering_pdf(r, rec, scattered_ray);
            //If pdf is 0 or close, stop. Generated impossible sample. Probably shouldn't happen!
            if (pdf_mat <= epsilon) return empty_path_result();

            color bsdf = rec.mat->bsdf(scatter_dir, rec, -r.direction());
            double cos_theta = fmax(0.0, dot(scatter_dir, rec.normal));
            //Throughput at this point (from a scattered direction, out at a ray direction)
            color throughput = bsdf * cos_theta / pdf_mat;

            child_state.prev_bsdf_pdf = pdf_mat;
            child_state.overall_throughput = throughput * child_state.overall_throughput;

            indirect_res = path_trace(scattered_ray, world, lights, sky, child_state);

            indirect_res.radiance_from_path = indirect_res.radiance_from_path * throughput;
        }

        out_result = indirect_res;
        out_result.radiance_from_path += color_from_light;
        return out_result;
    }


    path_state initial_path_state() const {
        path_state p_state;
        //Here depth starts from 0 and counts upwards
        p_state.depth = 0;
        p_state.overall_throughput = colors::white;
        p_state.prev_bsdf_pdf = 1.0;
        return p_state;
    }

    path_result color_path_result(color radiance) const {
        path_result res = empty_path_result();
        res.radiance_from_path = radiance;
        return res;
    }

    path_result empty_path_result() const {
        path_result out_result;
        out_result.radiance_from_path = color(0,0,0);
        out_result.terminated_on_light = false;
        return out_result;
    }
};

#endif //BENDERER_SIMPLE_PATH_TRACER_H