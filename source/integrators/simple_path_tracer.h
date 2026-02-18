//
// Created by binjabin on 1/17/26.
//

#ifndef BENDERER_SIMPLE_PATH_TRACER_H
#define BENDERER_SIMPLE_PATH_TRACER_H
#include "../scene/scene.h"
#include "../scene/material/surface_material.h"
#include "../records/path_result.h"
#include "../records/path_state.h"

class simple_path_tracer : public integrator {

public:
    simple_path_tracer(int max_depth)
        : m_max_depth(max_depth) {
    };

    // Correctly override integrator::ray_color (const-correct signature)
    color ray_color(const ray &r, int depth, const world& world) const {
        path_state p_state = initial_path_state();
        path_result res = path_trace(r, world, p_state);
        return res.radiance_from_path;
    }


private:
    int m_max_depth;

    path_result path_trace(const ray& r, const world& world, const path_state& p_state) const {

        path_result out_result = path_result();

        //---------------------------------------
        // Terminate if our throughput becomes 0 (or close)
        if (max_component(p_state.overall_throughput) < epsilon) {
            return empty_path_result();
        }

        //---------------------------------------
        // First check that our ray hits anything
        surface_hit_rec rec;
        interval ray_t = interval(epsilon, infinity);
        //If it doesn't, default to skybox
        if ( !world.m_surfaces->surface_hit( r, ray_t, rec ) ) {
            color col_from_sky = world.m_sky->sample_color(r.direction());
            return color_path_result(col_from_sky);
        }

        //---------------------------------------
        // Get surface emittance
        color color_from_light = rec.m_mat->emitted(r, rec, rec.get_u(), rec.get_v(), rec.get_p());

        //---------------------------------------
        // We have a max depth. Terminate if the next sample would exceed that depth.
        if (p_state.depth + 1 >= m_max_depth) {
            return color_path_result(color_from_light);
        }

        //---------------------------------------
        // Otherwise, calculate ray "scatter". Either absorb ray (just return emittance, no more bounces) or calculate the scatter direction

        surface_scatter_rec srec;
        bool scattered = rec.m_mat->scatter(r, rec, srec);

        //If we don't scatter, then we exit early
        if (!scattered) {
            path_result no_scatter = color_path_result(color_from_light);
            return no_scatter;
        }

        //----------------------------------------
        // If our ray is scattered, we take another step, an indirect sample
        // Generate a scatter direction, and calculate lighting from that direction down our ray

        path_result indirect_res = get_indirect_result(r, world, p_state, srec, rec);

        out_result = indirect_res;
        out_result.radiance_from_path += color_from_light;
        return out_result;
    }

    path_result get_indirect_result(const ray& r, const world& world, const path_state& p_state, const surface_scatter_rec& srec, const surface_hit_rec& rec) const {
        path_result indirect_res = empty_path_result();
        path_state child_state = p_state;
        child_state.depth++;
        if (srec.is_spec) {

            //prev bsdf is meaningless for specular. Just keep sensible.
            child_state.prev_bsdf_pdf = 1.0;

            //attenuation is like the throughput of spec surfaces
            child_state.overall_throughput = srec.bsdf * child_state.overall_throughput;
            indirect_res = path_trace(srec.s_ray, world, child_state);
            indirect_res.radiance_from_path = indirect_res.radiance_from_path * srec.bsdf;
        }
        else {
            vec3 scatter_dir = srec.s_ray.direction();
            //If pdf is 0 or close, stop. Generated impossible sample. Probably shouldn't happen!
            if (srec.w_pdf <= epsilon) return empty_path_result();

            //If a volume, this is meaningless, just carry on
            //TODO: We had volume cos-theta stuff here. We handle this differently now!
            double cos_theta = fmax(0.0, dot(scatter_dir, rec.get_normal()));
            //Throughput at this point (from a scattered direction, out at a ray direction)
            color throughput = srec.bsdf * cos_theta / srec.w_pdf;

            child_state.prev_bsdf_pdf = srec.w_pdf;
            child_state.overall_throughput = throughput * child_state.overall_throughput;

            indirect_res = path_trace(srec.s_ray, world, child_state);

            indirect_res.radiance_from_path = indirect_res.radiance_from_path * throughput;
        }

        return indirect_res;
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