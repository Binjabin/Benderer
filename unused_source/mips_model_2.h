//
// Created by binjabin on 12/26/25.
//

#ifndef BENDERER_MIPS_MODEL_2_H
#define BENDERER_MIPS_MODEL_2_H
#include "../source/integrators/integrator.h"
#include "../source/scene/material/material.h"
#include "../source/structures/path_result.h"
#include "../source/structures/path_state.h"

class mips_model_2 : public integrator {

public:
    mips_model_2(int max_depth, int rr_min_depth, int num_light_samples_per_bounce)
        : m_max_depth(max_depth), m_num_light_samples_per_bounce(num_light_samples_per_bounce), m_rr_min_depth(rr_min_depth) {
    };

    color ray_color(const ray &r, int depth, const hittable &world, const hittable &lights, const shared_ptr<skybox> sky) const override {
        path_state p_state = initial_path_state();
        path_result res = path_trace(r, world, lights, sky, p_state);
        return res.radiance_from_path;
    }


private:
    int m_max_depth;
    int m_num_light_samples_per_bounce;
    int m_rr_min_depth;


    path_result path_trace(const ray& r, const hittable& world, const hittable& lights, const shared_ptr<skybox> sky, path_state& p_state) const {
        path_result out_result = path_result();

        //---------------------------------------
        // We have a max depth. Terminate if we have exceeded that depth.

        if (p_state.depth >= m_max_depth) {
            return empty_path_result();
        }

        //---------------------------------------
        // First check that our ray hits anything

        hit_record rec;
        interval ray_t = interval(epsilon, infinity);
        if ( !world.hit( r, ray_t, rec ) ) {
            out_result.radiance_from_path = sky->sample_color(r.direction());
            out_result.terminated_on_light = false;

            return out_result;
        }

        //---------------------------------------
        // Our surface might have some emittance even if it isn't an explicit light.
        // So we calculate this in any case here.
        color color_from_light = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

        //---------------------------------------
        // Check if we have hit an explicit light source (ie a scene light)
        // If yes, our return value is calculated from the light's emission

        if ( rec.is_explicit_light ) {
            //TODO: Need to pass through the previous bsdf pdf in the p_state. Otherwise we end up over weighting bounces into lights
            double p_B = p_state.prev_bsdf_pdf;
            //TODO: Calculate the solid angle pdf value for the direction?
            double p_L = 1.0 / (4.0 * pi);
            double w = power_heuristic(p_B, p_L);
            w = 1.0;
            out_result.radiance_from_path = color_from_light * w;
            out_result.terminated_on_light = true;

            return out_result;
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

        //---------------------------------------
        // Then calculate (Except for specular materials) calculate direct lighting

        color direct_res = colors::black;
        if (!srec.skip_pdf) {
            direct_res = path_trace_direct(r, rec, world, lights, sky);
        }

        //----------------------------------------
        // Now apply russian roulette to early terminate low-throughput paths

        /*
        if (p_state.depth >= m_rr_min_depth) {
            double m_component = max_component(p_state.overall_throughput);
            double p_survival = fmin(0.95, m_component);

            if (random_double() > p_survival) {
                return color_path_result(color_from_light + direct_res);
            }

            //Otherwise scale the throughput (up) to account for the probability of not returning anything, so as not to introduce bias
            p_state.overall_throughput /= p_survival;
        }
        */

        //----------------------------------------
        // If we don't terminate through russian roulette, we take another step, an indirect sample
        // Generate a scatter direction, and calculate lighting from that direction down our ray

        vec3 scatter_dir = srec.skip_pdf ? srec.skip_pdf_ray.direction() : srec.pdf_ptr->generate();
        ray scattered_ray = ray(rec.p, scatter_dir, r.time());

        path_result indirect_res = path_trace_indirect(scattered_ray, r, rec, world, lights, sky, p_state);

        //---------------------------------------
        // Calculate return value by combining

        path_result final_res;
        final_res.radiance_from_path = color_from_light + direct_res + indirect_res.radiance_from_path;
        //TODO: Is this right? Do we need to know if a path eventually terminates on a light or only if it terminated on a light in the "next" step. Does this even make a difference?
        final_res.terminated_on_light = indirect_res.terminated_on_light;
        return final_res;
    }

    color path_trace_direct(const ray& in_ray, hit_record& rec, const hittable& world, const hittable& lights, const shared_ptr<skybox> sky) const {

        color total_direct_light = color(0,0,0);

        for (int i = 0; i < m_num_light_samples_per_bounce; i++) {

            //Sample a point on a scene light
            local_light_sample l_sample = sample_over_flux(lights, sky, rec.p);

            //Cast shadow ray towards the point
            hit_record shadow_rec;
            ray shadow_ray = ray(rec.p, l_sample.m_direction, rec.time);
            interval shadow_ray_interval = interval(epsilon, l_sample.m_distance - epsilon);
            bool obstructed = world.hit(shadow_ray, shadow_ray_interval, shadow_rec);

            if (!obstructed) {
                //If we weren't obstructed, then this light sample has a contribution
                vec3 shadow_ray_dir = shadow_ray.direction();

                //Weight by chance we scattered in this direction
                double mat_pdf = rec.mat->scattering_pdf(in_ray, rec, shadow_ray);
                //Geometry term for material
                double cos_mat = fmax(0.0, dot(shadow_ray_dir, rec.normal));

                //Get the pdf of the light
                double light_pdf = l_sample.m_pdf_w;

                //Reverse weight by BSDF function of material
                color bsdf = rec.mat->bsdf(shadow_ray_dir, rec, -in_ray.direction());

                //Geometry term for light is implicit within the light sample's pdf (since it is solid angle)
                color physical_contrib = bsdf * cos_mat * l_sample.m_radiance;

                double p_L = light_pdf;
                double p_B = mat_pdf;
                double w = power_heuristic(p_L, p_B);

                color contrib = colors::black;
                //Skip sample if pdf is too tiny (impossible or backface etc)
                if (light_pdf > epsilon) {
                    //w = 1.0;
                    //Downweight by sampling probability and MIS
                    contrib = physical_contrib * ( 1.0 / light_pdf ) * w;
                }

                total_direct_light += contrib;
            }
            //If we were in fact obstructed, this sample has zero contribution
        }

        //Average over all the samples
        return total_direct_light / m_num_light_samples_per_bounce;
    }

    path_result path_trace_indirect(const ray& scatter_r, const ray& in_r, hit_record& rec, const hittable& world, const hittable& lights, const shared_ptr<skybox> sky, path_state& p_state) const {

        //If a material for which we deterministic generate a scatter like a specular material, fallback to in-built direction
        //Otherwise, sample direction from pdf
        //TODO: this logic should probably be inside the material, ie a function to simply generate a bounce direction
        vec3 scatter_dir = scatter_r.direction();
        color bsdf = rec.mat->bsdf(scatter_dir, rec, -in_r.direction());

        double cos_theta = fmax(0.0, dot(scatter_dir, rec.normal));

        double pdf_mat = rec.mat->scattering_pdf(in_r, rec, scatter_r);

        //Do next step

        color throughput = bsdf * cos_theta / pdf_mat;

        path_state child_state = p_state;
        child_state.depth++;
        child_state.prev_bsdf_pdf = pdf_mat;
        //Overall throughput just used for Russian roulette
        child_state.overall_throughput = throughput * child_state.overall_throughput;
        path_result indirect_res =  path_trace(scatter_r, world, lights, sky, child_state);

        indirect_res.radiance_from_path = indirect_res.radiance_from_path * throughput;

        //TODO: MIS.. Downweight if we don't indirect bounce into a light?

        return indirect_res;
    }

    local_light_sample sample_over_flux(const hittable &lights, const shared_ptr<skybox> sky, point3& from) const {

        double sky_weight = sky->get_flux_weight();
        double lights_weight = lights.get_flux_weight();
        double flux_sum = sky_weight + lights_weight;

        double sample_seed = flux_sum * random_double();


        if (sample_seed < sky_weight) {
            //Sample from skybox
            double p = sky_weight / flux_sum;

            environment_light_sample light_sample = sky->sample_light_over_flux(p);
            local_light_sample local_sample = light_sample.to_local_sample(from);
            local_sample.m_pdf_w *= p;
            return local_sample;
        }

        double new_seed = (sample_seed - sky_weight) / lights_weight;
        double p = lights_weight / flux_sum;

        surface_light_sample light_sample = lights.sample_light_over_flux(new_seed, p);
        local_light_sample local_sample = light_sample.to_local_sample(from);
        local_sample.m_pdf_w *= p;
        return local_sample;
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

    //Balance pdf_a compared to pdf_b with power beta
    double power_heuristic(double pdf_a, double pdf_b, double beta = 2.0) const {
        double a = pow(pdf_a, beta);
        double b = pow(pdf_b, beta);
        return a / ( a + b );
    }

};

#endif //BENDERER_MIPS_MODEL_2_H