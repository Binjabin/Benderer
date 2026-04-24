//
// Created by binjabin on 1/25/26.
//

#ifndef BENDERER_MIS_MEDIUM_PATH_TRACER_H
#define BENDERER_MIS_MEDIUM_PATH_TRACER_H

#include "../records/medium_scatter_rec.h"
#include "../scene/scene.h"
#include "../scene/material/surface_material.h"
#include "../records/path_result.h"
#include "../records/path_state.h"
#include "../samplers/direct_light_sampler.h"
#include "../samplers/medium_sampler.h"

class mis_medium_path_tracer : public integrator {

public:
    mis_medium_path_tracer(int max_depth, int rr_start_depth, int direct_samples)
        : m_max_depth(max_depth), m_rr_start_depth(rr_start_depth), m_direct_samples(direct_samples) {
    };

    // Correctly override integrator::ray_color (const-correct signature)
    color ray_color(const ray &r, int depth, const world& world) const {
        path_state p_state = path_state::initial_path_state();
        path_result res = path_trace(r, world, p_state);
        return res.radiance_from_path;
    }


private:
    int m_max_depth;
    int m_rr_start_depth;
    int m_direct_samples;

    path_result path_trace(const ray& r, const world& world, const path_state& p_state) const {

        path_result out_result = path_result();


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

        if (p_state.depth > 0 && !p_state.prev_was_delta) {
                double nee_pdf = direct_light_sampler::pdf_w(world, p_state.prev_p, r.direction());
                col_from_sky *= mis_weight(p_state.prev_bsdf_pdf, m_direct_samples * nee_pdf);
            }

            col_from_sky *= transmittance;
            return path_result::color_path_result(col_from_sky);
        }

        //---------------------------------------
        // Scattered at some point in the media
        if (hit_medium) {
            //If we do hit the medium first, use this for scattering/absorption
            auto p = medium_rec.m_p();
            color emittance = medium_rec.m_emission;

            if (p_state.depth > 0 && !p_state.prev_was_delta) {
                double nee_pdf = direct_light_sampler::pdf_w(world, p_state.prev_p, r.direction());
                emittance *= mis_weight(p_state.prev_bsdf_pdf, m_direct_samples * nee_pdf);
            }

            //---------------------------------------
            // Terminate due to max depth termination!
            if (p_state.depth + 1 >= m_max_depth) {
                return path_result::color_path_result(emittance * medium_rec.m_transmittance);
            }

            double mat_inv_pdf = 1.0 / medium_rec.m_mat_pdf;

            if (!medium_rec.m_is_scatter) {
                //Absorb event
                return path_result::color_path_result(emittance * medium_rec.m_transmittance);
            }

            //---------------------------------------
            // Otherwise get NEE contrib

            color direct = evaluate_direct_medium(r, medium_rec, world);

            //---------------------------------------
            // And scatter in the medium!
            // First calculate scatter direction

            medium_scatter_rec srec;
            medium_rec.m_mat->scatter_is(-r.direction(), srec);
            ray sray = ray(p, srec.s_dir);

            //---------------------------------------
            // Then send out the next ray
            path_state child_state = p_state;
            child_state.depth++; // advance path depth for medium bounce
            child_state.prev_p = p;
            child_state.prev_bsdf_pdf = srec.w_pdf;
            child_state.prev_was_delta = false;

            color sigma_s = medium_rec.m_mat->sigma_s(medium_rec.m_local_p);
            double phase_factor = srec.phase_pdf / srec.w_pdf;

            // Update child throughput for path termination check

            child_state.overall_throughput *= (mat_inv_pdf * medium_rec.m_transmittance * sigma_s * phase_factor);

            //Apply russian roulette
            if (!russian_roulette(child_state)) {
                return path_result::color_path_result((emittance + direct * mat_inv_pdf) * medium_rec.m_transmittance);
            }

            path_result indirect_res = path_trace(sray, world, child_state);
            color indirect_rad = indirect_res.radiance_from_path * sigma_s * phase_factor * mat_inv_pdf;
            out_result.radiance_from_path = (indirect_rad + direct * mat_inv_pdf + emittance)  * medium_rec.m_transmittance;
            return out_result;
        }

        //---------------------------------------
        // Reached surface with no scatter
        color media_transmittance = intersect_medium ? medium_rec.m_transmittance : colors::white;

        //---------------------------------------
        // Get surface emittance
        color emission = rec.m_mat->emission(rec.m_intersection);


        // MIS weight the emission: if previous bounce was not a delta,
        // apply balance heuristic between BSDF sampling and NEE sampling.
        // For explicit lights only (to avoid double-counting with NEE).
        if (rec.m_is_explicit_light && p_state.depth > 0 && !p_state.prev_was_delta) {
            double nee_pdf = direct_light_sampler::pdf_w(world, p_state.prev_p, r.direction());
            emission *= mis_weight(p_state.prev_bsdf_pdf, m_direct_samples * nee_pdf);
        }

        //---------------------------------------
        // We have a max depth. Terminate if the next sample would exceed that depth.
        if (p_state.depth + 1 >= m_max_depth) {
            return path_result::color_path_result(emission * media_transmittance);
        }

        //---------------------------------------
        // Otherwise, calculate ray "scatter". Either absorb ray (just return emittance, no more bounces) or calculate the scatter direction

        surface_scatter_rec srec;
        bool scattered = rec.m_mat->scatter_is(rec.m_intersection, r.direction(), srec);

        //If we don't scatter, then we exit early
        if (!scattered) {
            path_result no_scatter = path_result::color_path_result(emission * media_transmittance);
            return no_scatter;
        }

        //----------------------------------------
        // If our ray is scattered, we take another step, an indirect sample
        // Generate a scatter direction, and calculate lighting from that direction down our ray
        path_state child_state = p_state;
        child_state.overall_throughput *= media_transmittance;
        child_state.prev_p = rec.get_p();

        color direct = srec.is_delta ? colors::black : evaluate_direct_surface(r, rec, world);

        path_result indirect_res = get_indirect_result(r, world, child_state, srec, rec);
        out_result = indirect_res;
        out_result.radiance_from_path += emission;
        out_result.radiance_from_path += direct;
        out_result.radiance_from_path *= media_transmittance;
        return out_result;
    }

    color evaluate_direct_surface(const ray& r, const surface_hit_rec& rec, const world& world) const {
        color direct = colors::black;

        const point3 p = rec.get_p();
        const vec3 normal = rec.get_normal();
        const vec3 wo = unit_vector(-r.direction());
        const auto mat = rec.m_mat;
        const auto isect = rec.m_intersection;

        for (int i = 0; i < m_direct_samples; i++) {
            local_light_sample ls;
            //If we fail to sample, leave
            if (!direct_light_sampler::sample_from_point(world, p, ls)) continue;

            //The chance we sampled this direction (based on picking lights)
            const double light_pdf_w = ls.m_pdf_w;
            if (light_pdf_w <= 0.0) continue;

            //Reject backfacing lights
            const vec3 wi = unit_vector(ls.m_direction);
            const double cos_theta = dot(wi, normal);
            if (cos_theta <= 0.0) continue;

            //Cast shadow ray until just before surface
            ray shadow_ray = ray(p, wi);
            interval shadow_ray_t = interval(epsilon, ls.m_distance - epsilon);

            //Check if the light point we sampled would ever reach our light
            if (!world.m_surfaces->surface_hit_check(shadow_ray, shadow_ray_t)) {
                //Path to light not blocked by m_surfaces, do media
                color shadow_transmittance = colors::white;
                medium_intersections shadow_medium_intersections;
                bool shadow_hit_medium = world.m_media->medium_hit(shadow_ray, shadow_ray_t, shadow_medium_intersections);
                if (shadow_hit_medium) {
                    shadow_transmittance = medium_sampler::uninterrupted_transmittance(shadow_ray, shadow_medium_intersections, shadow_ray_t);
                }

                const color light_emission = ls.m_radiance;
                const color bsdf_val = mat->bsdf(isect, wo, wi);
                const double bsdf_pdf = mat->pdf(isect, wo, wi);

                //Add result to direct:
                color contrib = light_emission * bsdf_val * cos_theta * shadow_transmittance / light_pdf_w;

                //Apply MIS
                double w = mis_weight(m_direct_samples * light_pdf_w, bsdf_pdf);

                color add = contrib * w;

                direct += add;
            }
            //Otherise we don't contribute
        }

        return direct / m_direct_samples;
    }

    color evaluate_direct_medium(const ray& r, const medium_hit_rec& rec, const world& world) const {
        color direct = colors::black;

        auto interaction = rec.m_interaction;
        point3 p = interaction.m_p;
        auto mat = rec.m_mat;

        for (int i = 0; i < m_direct_samples; i++) {
            local_light_sample ls;
            //If we fail to sample, leave
            if (!direct_light_sampler::sample_from_point(world, p, ls)) continue;

            //The chance we sampled this direction (based on picking lights)
            const double light_pdf_w = ls.m_pdf_w;
            if (light_pdf_w <= 0.0) continue;

            //Cast shadow ray until just before surface
            ray shadow_ray = ray(p, ls.m_direction);
            interval shadow_ray_t = interval(epsilon, ls.m_distance - epsilon);

            //Check if the light point we sampled would ever reach our light
            if (!world.m_surfaces->surface_hit_check(shadow_ray, shadow_ray_t)) {
                //Path to light not blocked by m_surfaces, do media
                color shadow_transmittance = colors::white;
                medium_intersections shadow_medium_intersections;
                bool shadow_hit_medium = world.m_media->medium_hit(shadow_ray, shadow_ray_t, shadow_medium_intersections);
                if (shadow_hit_medium) {
                    shadow_transmittance = medium_sampler::uninterrupted_transmittance(shadow_ray, shadow_medium_intersections, shadow_ray_t);
                }

                const color light_emission = ls.m_radiance;
                const double phase_val = mat->phase(interaction, -r.direction(), shadow_ray.direction());
                const double phase_pdf = phase_val;

                //Add result to direct:
                color sigma_s = mat->sigma_s(rec.m_local_p);
                color contrib = light_emission * sigma_s * phase_val * shadow_transmittance / light_pdf_w;

                //Apply MIS
                double w = mis_weight(m_direct_samples * light_pdf_w, phase_pdf);

                color add = contrib * w;

                direct += add;
            }
            //Otherise we don't contribute
        }

        return direct / m_direct_samples;
    }

    path_result get_indirect_result(const ray& r, const world& world, const path_state& p_state, const surface_scatter_rec& srec, const surface_hit_rec& rec) const {
        path_result indirect_res = path_result::empty_path_result();
        path_state child_state = p_state;
        child_state.depth++;
        if (srec.is_delta) {
            //----------------------------------------
            // We didn't sample a direction, so our throughput is just the bsdf value.
            // No cos term as it is considered to be "absorbed" into the bsdf for delta materials
            child_state.overall_throughput = srec.bsdf * child_state.overall_throughput;
            child_state.prev_was_delta = true;
            child_state.prev_bsdf_pdf = 1.0;

            if (!russian_roulette(child_state)) {
                return path_result::empty_path_result();
            }

            indirect_res = path_trace(srec.s_ray, world, child_state);
            indirect_res.radiance_from_path = indirect_res.radiance_from_path * srec.bsdf;
        }
        else {
            //----------------------------------------
            // Use the actual sampling PDF from scatter_is (cosine-weighted)
            vec3 scatter_dir = srec.s_ray.direction();
            double cos_theta = fmax(0.0, dot(scatter_dir, rec.get_normal()));
            double pdf = srec.w_pdf;
            color throughput = (pdf > 0.0) ? srec.bsdf * cos_theta / pdf : colors::black;

            //----------------------------------------
            // Keep track of overall throughput, to terminate paths with tiny contributions early
            child_state.overall_throughput *= throughput;
            child_state.prev_was_delta = false;
            child_state.prev_bsdf_pdf = pdf;

            if (!russian_roulette(child_state)) {
                return path_result::empty_path_result();
            }

            indirect_res = path_trace(srec.s_ray, world, child_state);
            indirect_res.radiance_from_path = indirect_res.radiance_from_path * throughput;
        }

        return indirect_res;
    }

    bool russian_roulette(path_state& p_state) const {
        if (p_state.depth < m_rr_start_depth) return true;

        double p = rr_importance(p_state.overall_throughput);

        if (random_double() > p) return false;

        p_state.overall_throughput /= p;
        return true;
    }

    double rr_importance(const vec3& throughput) const {
        return std::clamp(max_component(throughput), 0.05, 0.95);
    }

    static double mis_weight(double pdf_a, double pdf_b) {
        double a2 = pdf_a * pdf_a;
        double b2 = pdf_b * pdf_b;
        double sum = a2 + b2;
        if (sum <= 0.0) return 0.0;
        return a2 / (sum);
    }
};

#endif //BENDERER_MIS_MEDIUM_PATH_TRACER_H