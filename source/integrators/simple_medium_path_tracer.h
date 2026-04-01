//
// Created by binjabin on 1/25/26.
//

#ifndef BENDERER_SIMPLE_MEDIUM_PATH_TRACER_H
#define BENDERER_SIMPLE_MEDIUM_PATH_TRACER_H

#include "../records/medium_scatter_rec.h"
#include "../scene/scene.h"
#include "../scene/material/surface_material.h"
#include "../records/path_result.h"
#include "../records/path_state.h"
#include "../samplers/medium_sampler.h"

class simple_medium_path_tracer : public integrator {

public:
    simple_medium_path_tracer(int max_depth)
        : m_max_depth(max_depth) {
    };

    // Correctly override integrator::ray_color (const-correct signature)
    color ray_color(const ray &r, int depth, const world& world) const override {
        path_state p_state = path_state::initial_path_state();
        path_result res = path_trace(r, world, p_state);
        return res.radiance_from_path;
    }

private:
    int m_max_depth;

    path_result path_trace(const ray& r, const world& world, path_state& p_state) const {

        path_result out_result = path_result();

        //---------------------------------------
        // Terminate if our throughput becomes 0 (or close)
        if (max_component(p_state.overall_throughput) < epsilon) {
            return path_result::empty_path_result();
        }

        //---------------------------------------
        // Check for a surface hit
        surface_hit_rec rec;
        interval ray_t = interval(epsilon, infinity);
        bool hit_surface = world.m_surfaces->surface_hit( r, ray_t, rec );

        //---------------------------------------
        // Check for a medium hits

        double end_t = hit_surface ? rec.get_t() : infinity;
        interval medium_interval = interval(epsilon, end_t);

        //Check for media up to first surface
        medium_intersections medium_recs;
        bool intersect_medium = world.m_media->medium_hit(r, medium_interval, medium_recs);

        medium_hit_rec medium_rec;
        bool hit_medium = false;
        if (intersect_medium) {
            hit_medium = medium_sampler::sample_distance(r, medium_recs, medium_interval, medium_rec);
        }


        //---------------------------------------
        // If no intersection, default to skybox
        if (!(hit_surface || hit_medium)) {
            color col_from_sky = world.m_sky->sample_color(r.direction());

            const color media_transmittance = intersect_medium ? medium_rec.m_transmittance : colors::white;

            color res = col_from_sky * media_transmittance;
            return path_result::color_path_result(res);
        }

        //Sampled media hit up to first surface so any media hit will hit before surface
        if (hit_medium){
            //If we do hit the medium first, use this for scattering/absorption
            auto p = medium_rec.m_p();
            color emittance = medium_rec.m_mat ? medium_rec.m_mat->emission(p) : colors::black;

            //---------------------------------------
            // Terminate due to max depth termination!

            if (p_state.depth + 1 >= m_max_depth) {
                return path_result::color_path_result(emittance * medium_rec.m_transmittance);
            }

            //---------------------------------------
            // Terminate due to absorb event!
            if (!medium_rec.m_is_scatter) {
                return path_result::color_path_result(emittance * medium_rec.m_transmittance);
            }

            //---------------------------------------
            // Otherwise scatter in the medium!
            // First calculate scatter direction
            medium_scatter_rec srec;
            medium_rec.m_mat->scatter(-r.direction(), srec);
            ray sray = ray(p, srec.s_dir);

            //---------------------------------------
            // Then throughput

            //TODO: This weighting here is for MVP purposes
            const double pdf_scalar = medium_rec.m_transmittance_pdf_scalar;
            const double sigma_s_scalar = medium_rec.m_sigma_s_scalar;
            const double denom = std::max(epsilon, (pdf_scalar * sigma_s_scalar));
            color throughput = medium_rec.m_transmittance * medium_rec.m_sigma_s * (1.0 / denom);
            throughput *= (srec.phase_pdf / srec.w_pdf);

            //---------------------------------------
            // Then send out the next ray

            path_state child_state = p_state;
            child_state.depth++; // advance path depth for medium bounce
            child_state.overall_throughput *= throughput;

            path_result indirect_res = path_trace(sray, world, child_state);

            // Accumulate emission at the medium point (if any)
            out_result.radiance_from_path += (emittance * medium_rec.m_transmittance);
            out_result.radiance_from_path += indirect_res.radiance_from_path * throughput;

            return out_result;
        }

        //---------------------------------------
        // If we traversed media with no event, apply transmittance
        const color media_transmittance = intersect_medium ? medium_rec.m_transmittance : colors::white;

        //---------------------------------------
        // Get surface emittance
        color surface_emittance = rec.m_mat->emission(rec.m_intersection);

        //---------------------------------------
        // We have a max depth. Terminate if the next sample exceeds that depth.
        if (p_state.depth + 1 >= m_max_depth) {
            color res = surface_emittance * media_transmittance;
            return path_result::color_path_result(res);
        }

        //---------------------------------------
        // Otherwise, calculate ray "scatter". Either absorb ray (just return emittance, no more bounces) or calculate the scatter direction

        surface_scatter_rec srec;
        bool scattered = rec.m_mat->scatter_is(rec.m_intersection, r.direction(), srec);

        //If we don't scatter, then we exit early
        if (!scattered) {
            color res = surface_emittance * media_transmittance;
            path_result no_scatter = path_result::color_path_result(res);
            return no_scatter;
        }

        //----------------------------------------
        // If our ray is scattered, we take another step, an indirect sample
        // Generate a scatter direction, and calculate lighting from that direction down our ray

        path_result indirect_res = get_indirect_result(r, world, p_state, srec, rec);

        out_result = indirect_res;
        color from_surface = out_result.radiance_from_path + surface_emittance;
        from_surface = from_surface * media_transmittance;
        out_result.radiance_from_path = from_surface;
        return out_result;
    }

    path_result get_indirect_result(const ray& r, const world& world, const path_state& p_state, const surface_scatter_rec& srec, const surface_hit_rec& rec) const {
        path_result indirect_res = path_result::empty_path_result();
        path_state child_state = p_state;
        child_state.depth++;
        if (srec.is_delta) {

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
            if (srec.w_pdf <= epsilon) return path_result::empty_path_result();

            //If a volume, this is meaningless, just carry on
            double cos_theta = fmax(0.0, dot(scatter_dir, rec.get_normal()));
            //Throughput at this point (from a scattered direction, out at a ray direction)
            color throughput = srec.bsdf * cos_theta / srec.w_pdf;

            child_state.prev_bsdf_pdf = srec.w_pdf;
            child_state.overall_throughput = child_state.overall_throughput * throughput;

            indirect_res = path_trace(srec.s_ray, world, child_state);

            indirect_res.radiance_from_path = indirect_res.radiance_from_path * throughput;
        }

        return indirect_res;
    }
};

#endif //BENDERER_SIMPLE_MEDIUM_PATH_TRACER_H