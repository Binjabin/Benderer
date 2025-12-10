//
// Created by binjabin on 12/10/25.
//

#ifndef BENDERER_ONE_STEP_SHADOW_H
#define BENDERER_ONE_STEP_SHADOW_H

class one_step_shadow : public integrator {
public:
    const int n_light_samples_per_bounce = 4;

    color ray_color(const ray &r, int depth, const hittable &world, const hittable &lights, const shared_ptr<skybox> sky) const override {
        //no more light is gathered
        if ( depth <= 0 ) {
            return color( 0, 0, 0 );
        }

        hit_record rec;
        auto ray_interval = interval( 0.001, infinity );

        //Check our ray hits ANYTHING
        if ( !world.hit( r, ray_interval, rec ) ) {
            return sky->sample_color(r.direction());
        }

        //First calculate direct contribution from this point from emission
        color color_from_emission = rec.mat->emitted( r, rec, rec.u, rec.v, rec.p );

        color total_direct_light = color(0,0,0);
        for (int i = 0; i < n_light_samples_per_bounce; i++) {
            color this_sample_direct = sample_direct_light_for_hit(r, world, lights, sky, rec);
            total_direct_light += this_sample_direct;
        }
        color average_direct_light = total_direct_light / n_light_samples_per_bounce;

        return average_direct_light + color_from_emission;
    }

private:

    color sample_direct_light_for_hit(const ray &r, const hittable &world, const hittable &lights, const shared_ptr<skybox> sky, hit_record& rec) const {

        //Calculate light directly onto this point through samples of scene lights:
        shared_ptr<light_sample> light_sample = sample_over_flux(lights, sky, rec.p);

        hit_record shadow_rec;
        vec3 shadow_ray_dir = light_sample->light_direction(rec.p);
        ray shadow_ray = ray(rec.p, shadow_ray_dir, r.time());
        double shadow_ray_length = light_sample->light_distance(rec.p);
        //TODO: Improve?
        interval shadow_ray_interval = interval(0.001, shadow_ray_length - 0.001);
        //TODO: Improve? eg what about backface

        color indirect = color(0,0,0);
        if (!world.hit(shadow_ray, shadow_ray_interval, shadow_rec)) {
            indirect = light_sample->m_radiance;
        };

        double pdf_w = light_sample->pdf_w_value(rec.p);
        if (pdf_w <= 1e-12) {
            return color(0,0,0);
        }

        double cos_theta = fmax(0.0, dot(shadow_ray_dir, rec.normal));
        color atten = rec.mat->get_attenuation(rec);
        vec3 w_indirect = (atten * cos_theta * indirect) / pdf_w;
        return w_indirect;
    }

    shared_ptr<light_sample> sample_over_flux(const hittable &lights, const shared_ptr<skybox> sky, point3& from) const {
        double sky_weight = sky->get_flux_weight();
        double lights_weight = lights.get_flux_weight();
        double flux_sum = sky_weight + lights_weight;

        double sample = flux_sum * random_double();

        if (sample < sky_weight) {
            //Sample from sky
            double p = sky_weight / flux_sum;
            return sky->sample_light_over_flux(p);
        }
        double new_seed = (sample - sky_weight) / lights_weight;
        double p = lights_weight / flux_sum;
        return lights.sample_light_over_flux(new_seed, p);
    }


};

#endif //BENDERER_ONE_STEP_SHADOW_H