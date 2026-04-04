//
// Created by binjabin on 1/17/26.
//

#ifndef BENDERER_SIMPLE_MEDIUM_PATH_TRACER_H
#define BENDERER_SIMPLE_MEDIUM_PATH_TRACER_H
#include "../scene/scene.h"
#include "../scene/material/surface_material.h"
#include "../records/path_result.h"
#include "../records/path_state.h"

class simple_medium_path_tracer : public integrator {

public:
    simple_medium_path_tracer(int max_depth)
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
        // First check that our ray hits a surface
        surface_hit_rec rec;
        interval ray_t = interval(epsilon, infinity);
        bool hit_surface = world.m_surfaces->surface_hit( r, ray_t, rec );

        //---------------------------------------
        // Then check that our ray hits a volume
        double end_t = hit_surface ? rec.get_t() : infinity;
        interval medium_interval = interval(epsilon, end_t);
        //Check for media up to first surface
        medium_intersections medium_recs;
        bool intersect_medium = world.m_media->medium_hit(r, medium_interval, medium_recs);

        //Sample any media for scatters
        medium_hit_rec medium_rec;
        bool hit_medium = false;
        if (intersect_medium) {
            hit_medium = medium_sampler::sample_distance(r, medium_recs, medium_interval, medium_rec);
        }

        //---------------------------------------
        // If no intersection, default to skybox
        if ( !(hit_surface || hit_medium) ) {
            color col_from_sky = world.m_sky->sample_color(r.direction());
            color transmittance = intersect_medium ? medium_rec.m_transmittance : colors::white;
            col_from_sky *= transmittance;
            return color_path_result(col_from_sky);
        }

        //---------------------------------------
        // Scattered at some point in the media
        if (hit_medium) {
            //If we do hit the medium first, use this for scattering/absorption
            auto p = medium_rec.m_p();
            color emittance = medium_rec.m_emission;

            //---------------------------------------
            // Terminate due to max depth termination!
            if (p_state.depth + 1 >= m_max_depth) {
                return path_result::color_path_result(emittance * medium_rec.m_transmittance);
            }

            //---------------------------------------
            // Terminate due to absorb event (Shouldn't Happen)!
            if (!medium_rec.m_is_scatter) {
                return path_result::color_path_result(emittance * medium_rec.m_transmittance);
            }

            double mat_inv_pdf = 1.0 / medium_rec.m_mat_pdf;

            //---------------------------------------
            // Otherwise scatter in the medium!
            // First calculate scatter direction
            medium_scatter_rec srec;
            medium_rec.m_mat->scatter(-r.direction(), srec);
            ray sray = ray(p, srec.s_dir);

            //---------------------------------------
            // Then send out the next ray
            path_state child_state = p_state;
            child_state.depth++; // advance path depth for medium bounce

            color sigma_s = medium_rec.m_mat->sigma_s(p);
            double phase_factor = srec.phase_pdf / srec.w_pdf;

            // Update child throughput for path termination check

            child_state.overall_throughput *= (mat_inv_pdf * medium_rec.m_transmittance * sigma_s * phase_factor);

            path_result indirect_res = path_trace(sray, world, child_state);
            color indirect_rad = indirect_res.radiance_from_path;
            out_result.radiance_from_path = (indirect_rad * sigma_s * phase_factor * mat_inv_pdf + emittance)  * medium_rec.m_transmittance;
            return out_result;
        }

        //---------------------------------------
        // Reached surface with no scatter
        color media_transmittance = intersect_medium ? medium_rec.m_transmittance : colors::white;

        //---------------------------------------
        // Get surface emittance
        color emission = rec.m_mat->emission(rec.m_intersection);

        //---------------------------------------
        // We have a max depth. Terminate if the next sample would exceed that depth.
        if (p_state.depth + 1 >= m_max_depth) {
            return color_path_result(emission * media_transmittance);
        }

        //---------------------------------------
        // Otherwise, calculate ray "scatter". Either absorb ray (just return emittance, no more bounces) or calculate the scatter direction

        surface_scatter_rec srec;
        bool scattered = rec.m_mat->scatter(rec.m_intersection, r.direction(), srec);

        //If we don't scatter, then we exit early
        if (!scattered) {
            path_result no_scatter = color_path_result(emission * media_transmittance);
            return no_scatter;
        }

        //----------------------------------------
        // If our ray is scattered, we take another step, an indirect sample
        // Generate a scatter direction, and calculate lighting from that direction down our ray
        path_state child_state = p_state;
        child_state.overall_throughput *= media_transmittance;

        path_result indirect_res = get_indirect_result(r, world, child_state, srec, rec);
        out_result = indirect_res;
        out_result.radiance_from_path += emission;
        out_result.radiance_from_path *= media_transmittance;
        return out_result;
    }

    path_result get_indirect_result(const ray& r, const world& world, const path_state& p_state, const surface_scatter_rec& srec, const surface_hit_rec& rec) const {
        path_result indirect_res = empty_path_result();
        path_state child_state = p_state;
        child_state.depth++;
        if (srec.is_delta) {
            //----------------------------------------
            // We didn't sample a direction, so our throughput is just the bsdf value.
            // No cos term as it is considered to be "absorbed" into the bsdf for delta materials 
            child_state.overall_throughput = srec.bsdf * child_state.overall_throughput;
            indirect_res = path_trace(srec.s_ray, world, child_state);
            indirect_res.radiance_from_path = indirect_res.radiance_from_path * srec.bsdf;
        }
        else {
            //----------------------------------------
            // Our sample is over a uniform hemisphere, so our pdf is just 1/2pi.
            // The cosine term is the geometry term for the material
            vec3 scatter_dir = srec.s_ray.direction();
            double cos_theta = fmax(0.0, dot(scatter_dir, rec.get_normal()));
            double pdf = 1.0 / (2.0 * pi);
            color throughput = srec.bsdf * cos_theta / pdf;

            //----------------------------------------
            // Keep track of overall throughput, to terminate paths with tiny contributions early
            child_state.overall_throughput *= throughput;
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

#endif //BENDERER_SIMPLE_MEDIUM_PATH_TRACER_H