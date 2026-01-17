//
// Created by binjabin on 12/9/25.
//

#ifndef BENDERER_MIPS_MODEL_H
#define BENDERER_MIPS_MODEL_H
#include <algorithm>

#include "../source/integrators/integrator.h"
#include "../source/structures/scatter_record.h"
#include "../source/scene/material/material.h"

class mips_model : public integrator {

private:
    int m_max_depth;
    int m_num_light_samples_per_bounce;
    int m_rr_min_depth;

public:

    mips_model(int max_depth, int rr_min_depth, int num_light_samples_per_bounce)
        : m_max_depth(max_depth), m_num_light_samples_per_bounce(num_light_samples_per_bounce), m_rr_min_depth(rr_min_depth)
    {
    }

    color ray_color(const ray &r, int depth, const hittable &world, const hittable &lights, const shared_ptr<skybox> sky) const override {
        return ray_color_rr(r, depth, world, lights, sky, color(1,1,1));
    }

    color ray_color_rr(const ray &r, int depth, const hittable &world, const hittable &lights, const shared_ptr<skybox> sky, color rr_throughput) const {
        //no more light is gathered
        if ( depth <= 0 ) {
            return color( 0, 0, 0 );
        }

        hit_record rec;
        auto ray_interval = interval( epsilon, infinity );

        //===========================
        //Check our ray hits ANYTHING
        if ( !world.hit( r, ray_interval, rec ) ) {
            return sky->sample_color(r.direction());
        }

        //=================================================================
        // First calculate direct contribution from this point from emission
        // We now only do this for first step, all other light is found through direct light samples

        color color_from_emission = rec.mat->emitted( r, rec, rec.u, rec.v, rec.p );

        //Need to apply MIS to this emission here (unless of course we hit the light straight away)
        //Our structure prevents this from being straight forward so we just ignore emission unless sampled directly now

        if (depth < m_max_depth && rec.is_explicit_light) {
            color_from_emission = color(0, 0, 0);
        }

        //====================================
        // Then we check our scatter to see if our bounce is deterministic
        // If our bounce is, ie for specular/dieletric, then we don't need to cast shadow rays, just bounce
        // In this case we simply check if we scatter, and if we do, scatter deterministicly

        scatter_record srec;
        bool scattered = rec.mat->scatter(r, rec, srec);
        if (srec.skip_pdf) {
            if (scattered) {
                return srec.attenuation * ray_color(srec.skip_pdf_ray, depth - 1, world, lights, sky);
            }
            //Otherwise we just return emisive color (weird?)
            return color_from_emission;
        }

        //====================================
        // If we are on a light, we shouldn't perform any direct samples as they might end up on the same light - problematic
        // We treat this the same as an obscured branch, ie same as if all were occluded.
        // TODO: Consider improvements
        // Otherwise we sample direct light

        color average_direct_light = color(0,0,0);
        if (!rec.is_explicit_light) {
            color total_direct_light = color(0,0,0);
            for (int i = 0; i < m_num_light_samples_per_bounce; i++) {

                //Calculate light directly onto this point through samples of scene lights:
                shared_ptr<local_light_sample> light_sample = sample_over_flux(lights, sky, rec.p);

                hit_record shadow_rec;

                vec3 shadow_ray_dir = light_sample->m_direction;
                ray shadow_ray = ray(rec.p, shadow_ray_dir, r.time());

                //Values for MIS/pdfs
                double sky_weight = sky->get_flux_weight();
                double light_weight = lights.get_flux_weight();
                double flux_sum = sky_weight + light_weight;

                color direct_of_sample = color(0,0,0);

                //First handle skybox sample case:
                if (light_sample->m_is_env_light) {
                    interval shadow_ray_interval = interval(epsilon, infinity);

                    //If our doesn't hit anything between start and light, we contribute
                    if (!world.hit(shadow_ray, shadow_ray_interval, shadow_rec)) {

                        double p_of_env = sky_weight / flux_sum;
                        double pdf_of_d = p_of_env * sky->get_pdf_value(shadow_ray_dir);

                        //This is where we get contribution from skybox
                        color sky_direct = sky->sample_color(shadow_ray_dir);

                        //Calculate and weight by the chance we scattered in this direction
                        double mat_pdf = rec.mat->scattering_pdf(r, rec, shadow_ray);
                        //Geometry term, material
                        double cos_theta = fmax(0.0, dot(shadow_ray_dir, rec.normal));

                        color bsdf = rec.mat->bsdf(shadow_ray_dir, rec, -r.direction());

                        //Calculate contribution
                        color contrib = sky_direct * bsdf * cos_theta / pdf_of_d;

                        //MIS weight between chance this was a light selection (it was) and chance it was a material selection
                        double a2 = pdf_of_d * pdf_of_d;
                        double b2 = mat_pdf * mat_pdf;
                        double w_direct = (a2 + b2) > 0 ? (a2 / (a2 + b2)) : 0;

                        //Accumulate result
                        direct_of_sample = w_direct * contrib;
                    }
                    //Otherwise we keep our 0 contribution
                }
                else {
                    //Physical light
                    double shadow_ray_length = light_sample->m_distance;

                    //If our doesn't hit anything between start and light, we contribute
                    interval shadow_ray_interval = interval(epsilon, shadow_ray_length - epsilon);
                    if (!world.hit(shadow_ray, shadow_ray_interval, shadow_rec)) {

                        double p_of_light = light_weight / flux_sum;
                        double pdf_of_p = p_of_light * light_sample->m_pdf_w;

                        if (pdf_of_p <= 1e-8) {
                            std::clog << "PDF VALUE SMALL: " << std::endl;
                        }

                        //What color our light is
                        color light_direct = light_sample->m_radiance;

                        //pdf of mat, weight by chance we scattered in this direction
                        double mat_pdf = rec.mat->scattering_pdf(r, rec, shadow_ray);

                        if (mat_pdf <= 1e-8) {
                            std::clog << "PDF VALUE SMALL: " << std::endl;
                        }

                        //Geometry term, ie adjusting weight because of physical light


                        //Geometry term, material
                        double cos_theta = fmax(0.0, dot(shadow_ray_dir, rec.normal));

                        //BSDF
                        color bsdf = rec.mat->bsdf(shadow_ray_dir, rec, -r.direction());

                        //convert
                        color contrib = bsdf * light_direct * cos_theta * (1.0 / pdf_of_p) * light_sample->m_geometry_term;

                        //Do MIPS weighting
                        double pdf_of_d = p_of_light * light_sample->m_pdf_w;
                        double a2 = pdf_of_d * pdf_of_d;
                        double b2 = mat_pdf * mat_pdf;

                        if (a2 + b2 <= 1e-8) {
                            std::clog << "PDF VALUE SMALL: " << std::endl;
                        }

                        double w_direct = (a2 + b2) > 0 ? (a2 / (a2 + b2)) : 0;

                        direct_of_sample = w_direct * contrib;
                    }
                    //Otherwise we keep our 0 contribution
                }

                total_direct_light += direct_of_sample;

            }

            if (total_direct_light.length() > 10.0) {
                //std::clog << "BIG COLOR" << std::endl;
            }
            average_direct_light = total_direct_light / m_num_light_samples_per_bounce;
        }

        //================================================================================
        // Then we do our indirect step.
        // If we don't scatter, we just return the others
        // Then randomly choose a bounce ray along which to calculate indirect contribution

        if (!scattered) return color_from_emission + average_direct_light;

        shared_ptr<pdf> scatter_pdf = srec.pdf_ptr;
        vec3 scatter_dir = scatter_pdf->generate();

        ray scattered_ray = ray(rec.p, scatter_dir, r.time());

        color bsdf = rec.mat->bsdf(scatter_dir, rec, -r.direction());

        double cos_theta = fmax(0.0, dot(scatter_dir, rec.normal));

        double pdf_mat = rec.mat->scattering_pdf(r, rec, scattered_ray);

        if (pdf_mat < 1e-6) {
            std::clog << "SMALL PDF" << std::endl;
        }

        color throughput = bsdf * cos_theta / pdf_mat;

        //====================================
        // Check for early termination through russian roulette

        rr_throughput = rr_throughput * throughput;

        bool rr_continue = russian_roulette(rr_throughput, depth);
        if (!rr_continue) {
            return color_from_emission + average_direct_light;
        }


        color indirect_contribution = ray_color_rr(scattered_ray, depth - 1, world, lights, sky, rr_throughput);
        color color_from_indirect = indirect_contribution * throughput;

        //We don't use MIS weight to downweight a sample if we don't hit a light pdf.
        double w_indirect = 1.0;

        hit_record shadow_rec;
        ray shadow_test(rec.p, scatter_dir, r.time());

        if (lights.hit(shadow_test, interval(epsilon, infinity), shadow_rec)) {
            double light_pdf = compute_light_pdf_value(lights, sky, rec.p, scatter_dir);

            double a2 = light_pdf * light_pdf;
            double b2 = pdf_mat * pdf_mat;
            w_indirect = (a2 + b2) > 0 ? (b2 / (a2 + b2)) : 0;

            if (color_from_indirect.length() > 10.0) {
                std::clog << "FIREFLY DEBUG:"
                          << " indirect=" << indirect_contribution.length()
                          << " light_pdf=" << light_pdf
                          << " mat_pdf=" << pdf_mat
                          << " w_indirect=" << w_indirect
                          << " RATIO=" << (pdf_mat / light_pdf)
                          << std::endl;
            }
        }
        //ELSE we are in a multip-bounce path and don't downweight for MIS here

        color weighted_indirect = w_indirect * color_from_indirect;

        color result = color_from_emission + average_direct_light + weighted_indirect;

        if (depth == 8 && result.length() > 10.0) {
            std::clog << "BIG COLOR: " << pdf_mat << std::endl;
        }

        return result;
    }



private:

    //This checks whether to continue, and scales throughput vector appropriately if we do
    bool russian_roulette(color& throughput, int depth) const {
        //always continue before min depth
        if (depth < m_rr_min_depth) return true;

        float p = std::clamp(luminance(throughput), 0.0, 0.95);

        if (random_double() > p) {
            //Terminate
            return false;
        }
        throughput /= p;
        return true;
    }

    //This is the pdf for our direct light selection for an input direction.
    //The chance we get direct light contribution from this direction
    //ie if occluded, 0
    //if un-occluded, the
    double compute_light_pdf_value(const hittable& lights, const shared_ptr<skybox> sky, const point3& p, const vec3& d) const {
        ray r(p, d);
        hit_record rec;

        double sky_weight = sky->get_flux_weight();
        double light_weight = lights.get_flux_weight();
        double flux_sum = sky_weight + light_weight;
        double p_light = light_weight / flux_sum;
        double p_sky = sky_weight / flux_sum;


        if (!lights.hit(r, interval(epsilon, infinity), rec)) {
            //SKYBOX CASE
            //TODO: Address this case
            double pdf_w = sky->get_pdf_value(d);

            if (pdf_w <= 0.0) return 0.0;

            return pdf_w * p_sky;
        }

        //Light case
        auto pdf_v = rec.pdf_v;
        auto hit_point = rec.p;
        double dist2 = (p - hit_point).length_squared();
        vec3 unit_d = unit_vector(d);
        double cos_at_light = fmax(0, dot(-unit_d, rec.normal));

        if (pdf_v <= 0.0) return 0.0;
        if (cos_at_light <= 0.0) return 1e-10 * p_light;
        if (dist2 <= 0.0) return 0.0;
        auto pdf_w = pdf_v * dist2 / cos_at_light;

        return pdf_w * p_light;

    }

    shared_ptr<local_light_sample> sample_over_flux(const hittable &lights, const shared_ptr<skybox> sky, point3& from) const {
        double sky_weight = sky->get_flux_weight();
        double lights_weight = lights.get_flux_weight();
        double flux_sum = sky_weight + lights_weight;

        double sample = flux_sum * random_double();

        if (sample < sky_weight) {
            //Sample from sky
            double p = sky_weight / flux_sum;
            auto nl_sample = sky->sample_light_over_flux(p);
            return make_shared<local_light_sample>(nl_sample.to_local_sample(from));
        }
        double new_seed = (sample - sky_weight) / lights_weight;
        double p = lights_weight / flux_sum;
        auto nl_sample = lights.sample_light_over_flux(new_seed, p);
        return make_shared<local_light_sample>(nl_sample.to_local_sample(from));
    }
};

#endif //BENDERER_MIPS_MODEL_H