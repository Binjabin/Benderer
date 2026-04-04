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
    mis_medium_path_tracer(int max_depth, int rr_depth, int direct_samples)
        : m_max_depth(max_depth), m_rr_start_depth(rr_depth), m_direct_samples(direct_samples) {
    };

    // Correctly override integrator::ray_color (const-correct signature)
    color ray_color(const ray &r, int depth, const world& world) const override {
        path_state p_state = path_state::initial_path_state();
        path_result res = path_trace(r, world, p_state);

        return res.radiance_from_path;
    }


private:
    const int m_max_depth;
    const int m_rr_start_depth;
    const int m_direct_samples;
    const interval rr_prob = interval(0.05, 0.95);

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

            if (p_state.last_bounce_used_nee) {
                const double light_pdf_w = direct_light_sampler::pdf_w(world, r.origin(), r.direction());
                const double bsdf_pdf = p_state.prev_bsdf_pdf;
                const double w = power_heuristic(bsdf_pdf, m_direct_samples * light_pdf_w);
                col_from_sky *= w;
            }

            color res = col_from_sky * media_transmittance;
            return path_result::color_path_result(res);
        }

        //Sampled media hit up to first surface so any media hit will hit before surface
        if (hit_medium){
            //If we do hit the medium first, use this for scattering/absorption
            auto p = medium_rec.m_p();
            color emittance = medium_rec.m_mat ? medium_rec.m_mat->emission(p) : colors::black;

            //---------------------------------------
            // Sample direct

            color direct = get_medium_direct_result(r, medium_rec, world);

            //---------------------------------------
            // Terminate due to max depth termination!

            if (p_state.depth + 1 >= m_max_depth) {
                return path_result::color_path_result((direct + emittance) * medium_rec.m_transmittance);
            }

            //---------------------------------------
            // Potentially terminate due to russian roulette!

            if (p_state.depth >= m_rr_start_depth) {
                const double p = rr_prob.clamp(max_component(p_state.overall_throughput));
                double u = random_double();
                if (u > p) return path_result::color_path_result((direct + emittance) * medium_rec.m_transmittance);
                p_state.overall_throughput /= p;
            }

            //---------------------------------------
            // Otherwise scatter in the medium!
            // First calculate scatter direction
            medium_scatter_rec srec;
            medium_rec.m_mat->scatter_is(-r.direction(), srec);
            ray sray = ray(p, srec.s_dir);

            //---------------------------------------
            // Then throughput

            color throughput = medium_rec.m_transmittance * medium_rec.m_mat->sigma_s(p);
            throughput *= (srec.phase_pdf / srec.w_pdf);

            //---------------------------------------
            // Then send out the next ray

            path_state child_state = p_state;
            child_state.depth++; // advance path depth for medium bounce
            child_state.overall_throughput *= throughput;
            child_state.prev_bsdf_pdf = srec.w_pdf;
            child_state.last_bounce_used_nee = true;

            path_result indirect_res = path_trace(sray, world, child_state);

            // Accumulate emission at the medium point (if any)
            color contrib = ((direct + emittance) * medium_rec.m_transmittance) + indirect_res.radiance_from_path * throughput;

            out_result.radiance_from_path += contrib;

            return out_result;
        }

        //---------------------------------------
        // If we traversed media with no event, apply transmittance
        const color media_transmittance = intersect_medium ? medium_rec.m_transmittance : colors::white;

        //---------------------------------------
        // Get surface emittance

        //TODO: This part isn't correct now. We aren't downweighting for MIS at the right place. Better than nothing though..
        //TODO: Only truly fixed with bidirectional path tracing!
        color surface_emittance = rec.m_mat->emission(rec.m_intersection);
        if (rec.m_is_explicit_light && p_state.last_bounce_used_nee) {
            const double light_pdf_w = direct_light_sampler::pdf_w(world, r.origin(), r.direction());
            const double bsdf_pdf = p_state.prev_bsdf_pdf;
            const double w = power_heuristic(bsdf_pdf, m_direct_samples * light_pdf_w);
            surface_emittance *= w;
        }

        //---------------------------------------
        // Get direct lighting (Surface NEE)
        const bool used_nee_here = (!rec.m_is_explicit_light && !rec.m_mat->is_delta());
        color direct = colors::black;
        if (used_nee_here) {
            direct = get_surface_direct_result(r, rec, world);
        }

        //---------------------------------------
        // We have a max depth. Terminate if the next sample exceeds that depth.
        if (p_state.depth + 1 >= m_max_depth) {
            color res = (surface_emittance + direct) * media_transmittance;
            return path_result::color_path_result(res);
        }

        //---------------------------------------
        // Otherwise, calculate ray "scatter". Either absorb ray (just return emittance, no more bounces) or calculate the scatter direction

        surface_scatter_rec srec;
        bool scattered = rec.m_mat->scatter_is(rec.m_intersection, r.direction(), srec);

        //If we don't scatter, then we exit early
        if (!scattered) {
            color res = (surface_emittance + direct) * media_transmittance ;
            path_result no_scatter = path_result::color_path_result(res);
            return no_scatter;
        }

        //----------------------------------------
        // If our ray is scattered, we take another step, an indirect sample
        // Generate a scatter direction, and calculate lighting from that direction down our ray

        path_state child_state = p_state;
        child_state.overall_throughput *= media_transmittance;

        path_result indirect_res = get_indirect_result(r, world, child_state, srec, rec, used_nee_here);

        out_result = indirect_res;
        color from_surface = out_result.radiance_from_path + surface_emittance + direct;
        from_surface = from_surface * media_transmittance;
        out_result.radiance_from_path = from_surface;
        out_result.terminated_on_light = out_result.terminated_on_light || rec.m_is_explicit_light;


        /*
        if (max_component(from_surface) > 10.0) {
            std::clog << "HIGH depth=" << p_state.depth
                      << " emittance=" << max_component(surface_emittance)
                      << " direct=" << max_component(direct)
                      << " indirect=" << max_component(out_result.radiance_from_path)
                      << " last_nee=" << p_state.last_bounce_used_nee
                      << " is_light=" << rec.m_is_explicit_light
                        << " is_delta=" << rec.m_mat->is_delta()
                        << " bsdf_pdf=" << srec.w_pdf
                      << std::endl;
        }
        */

        return out_result;
    }

    path_result get_indirect_result(const ray& r, const world& world, const path_state& p_state, const surface_scatter_rec& srec, const surface_hit_rec& rec, const bool used_nee) const {
        path_result indirect_res = path_result::empty_path_result();
        path_state child_state = p_state;
        child_state.depth++;
        child_state.last_bounce_used_nee = used_nee;
        if (srec.is_delta) {

            //prev bsdf is meaningless for specular. Just keep sensible.
            child_state.prev_bsdf_pdf = p_state.prev_bsdf_pdf;
            child_state.last_bounce_used_nee = p_state.last_bounce_used_nee;
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

    color get_surface_direct_result(const ray& r, const surface_hit_rec& rec, const world& world) const {
        color direct = colors::black;

        auto isect = rec.m_intersection;
        point3 p = isect.get_p();
        vec3 n = isect.m_normal;
        auto mat = rec.m_mat;

        for (int i = 0; i < m_direct_samples; i++) {
            local_light_sample ls;
            //If we fail to sample, leave
            if (!direct_light_sampler::sample_from_point(world, p, ls)) break;

            //TODO: Move to seperate method?
            //Cast shadow ray until just before surface
            ray shadow_ray = ray(p, ls.m_direction);
            interval shadow_ray_t = interval(epsilon, ls.m_distance - epsilon);

            //Check if the light point we sampled would ever reach our light
            if (!world.m_surfaces->surface_hit_check(shadow_ray, shadow_ray_t)) {
                //Path to light not blocked by surfaces, do media
                color shadow_transmittance = colors::white;
                medium_intersections shadow_medium_intersections;
                bool shadow_hit_medium = world.m_media->medium_hit(shadow_ray, shadow_ray_t, shadow_medium_intersections);
                if (shadow_hit_medium) {
                    shadow_transmittance = medium_sampler::uninterupted_transmittance(shadow_ray, shadow_medium_intersections, shadow_ray_t);
                }

                //Geometry term for material
                const double cos_mat = fmax(0.0, dot(shadow_ray.direction(), n));

                const color light_emission = ls.m_radiance;
                const color bsdf = mat->bsdf(isect, -r.direction(), shadow_ray.direction());

                //The chance we sampled this direction (based on picking lights)
                const double light_pdf_w = ls.m_pdf_w;
                if (light_pdf_w <= 0.0) continue;

                //Add result to direct:
                color contrib = light_emission * bsdf * shadow_transmittance * cos_mat / light_pdf_w;

                //Apply MIS
                double bsdf_pdf = mat->pdf(isect, -r.direction(), shadow_ray.direction());
                double w = power_heuristic(m_direct_samples * light_pdf_w, bsdf_pdf);


                //DEBUG

                color add = contrib * w;

                direct += add;
            }
            //Otherise we don't contribute
        }

        if (!isfinite(direct.length())) {
            //HERE
            std::clog << "NON-FINITE DIRECT" << std::endl;
        }

        return direct / m_direct_samples;
    }

    color get_medium_direct_result(const ray& r, const medium_hit_rec& rec, const world& world) const {
        color direct = colors::black;

        auto interaction = rec.m_interaction;
        point3 p = interaction.m_p;
        auto mat = rec.m_mat;

        for (int i = 0; i < m_direct_samples; i++) {
            local_light_sample ls;
            //If we fail to sample, leave
            if (!direct_light_sampler::sample_from_point(world, p, ls)) break;

            //Cast shadow ray until just before surface
            ray shadow_ray = ray(p, ls.m_direction);
            interval shadow_ray_t = interval(epsilon, ls.m_distance - epsilon);

            //Check if the light point we sampled would ever reach our light
            if (!world.m_surfaces->surface_hit_check(shadow_ray, shadow_ray_t)) {
                //Path to light not blocked by surfaces, do media
                color shadow_transmittance = colors::white;
                medium_intersections shadow_medium_intersections;
                bool shadow_hit_medium = world.m_media->medium_hit(shadow_ray, shadow_ray_t, shadow_medium_intersections);
                if (shadow_hit_medium) {
                    shadow_transmittance = medium_sampler::uninterupted_transmittance(shadow_ray, shadow_medium_intersections, shadow_ray_t);
                }

                const color light_emission = ls.m_radiance;
                const double phase = mat->phase(interaction, -r.direction(), shadow_ray.direction());

                //The chance we sampled this direction (based on picking lights)
                const double light_pdf_w = ls.m_pdf_w;
                if (light_pdf_w <= 0.0) continue;


                //Add result to direct:
                color contrib = light_emission * phase * shadow_transmittance / light_pdf_w;

                //Apply MIS
                double phase_pdf = mat->phase(interaction, -r.direction(), shadow_ray.direction());
                double w = power_heuristic(m_direct_samples * light_pdf_w, phase_pdf);

                color add = contrib * w;

                const double add_mag = max_component(add);
                if (!std::isfinite(add_mag) || add_mag > 1e2) {
                    std::clog
                        << "\nFIRELLY (medium direct): add_mag=" << add_mag
                        << " light_pdf_w=" << light_pdf_w
                        << " phase_pdf=" << phase_pdf
                        << " is_env=" << ls.m_is_env_light
                        << std::endl;
                }

                direct += add;
            }
            //Otherise we don't contribute
        }

        if(!isfinite(direct.length())) {
            std::clog << "NON FINITE DIRECT" << std::endl;
        }

        return direct / m_direct_samples;

    }

    double power_heuristic(double a, double b) const {
        if (!std::isfinite(a) || !std::isfinite(b)) return 0.0;
        if (a <= 0.0) return 0.0;
        if (b <= 0.0) return 1.0;

        double a2 = a * a;
        double b2 = b * b;
        return a2 / (a2 + b2);
    }
};

#endif //BENDERER_MIS_MEDIUM_PATH_TRACER_H