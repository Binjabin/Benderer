//
// Created by binjabin on 1/30/26.
//

#ifndef BENDERER_MEDIUM_SAMPLER_H
#define BENDERER_MEDIUM_SAMPLER_H
#include <complex>
#include <memory>

#include "../scene/material/medium_material.h"
#include "../records/medium_intersection.h"


class medium_sampler {
public:
    static inline double sigma_scalar(const color& c) {
        return max_component(c);
    }

    static color slice_transmittance(const medium_slice& slice) {
        return exp(-slice.maj_optical_thickness);
    }

    static color slice_transmittance(const medium_slice& slice, double t) {
        return exp(-slice.m_sigma_maj * t);
    }

    static double slice_transmittance_for_channel(const medium_slice& slice, int channel) {
        return std::exp(-slice.maj_optical_thickness[channel]);
    }

    static double slice_transmittance_for_channel(const medium_slice& slice, double t, int channel) {
        return std::exp(-slice.m_sigma_maj[channel] * t);
    }

    static bool sample_distance(const ray& r, medium_intersections& intersections, const interval& t, medium_hit_rec& rec) {

        std::vector<medium_slice> slices;
        intersections.get_cropped_slices(t, slices);

        //Calculate total optical thickness
        color total_optical_thickness = colors::black;
        for (const medium_slice& slice : slices) {
            total_optical_thickness += slice.maj_optical_thickness;
        }
        double sum = total_optical_thickness[0] + total_optical_thickness[1] + total_optical_thickness[2];
        //No thickness!
        if (sum <= 0.0) {
            rec.m_transmittance = colors::white;
            rec.m_is_scatter = false;
            rec.m_mat = nullptr;
            rec.m_mat_pdf = 1.0;
            return false;
        }

        //Mix uniform with optical thickness weighted
        const double uniform_prob = 1.0 / 3.0;
        const double tau_weight = 1.0 - uniform_prob;

        double r_prop = tau_weight * (total_optical_thickness[0] / sum) + uniform_prob / 3.0;
        double g_prop = tau_weight * (total_optical_thickness[1] / sum) + uniform_prob / 3.0;
        double b_prop = tau_weight * (total_optical_thickness[2] / sum) + uniform_prob / 3.0;

        //Randomly select hero channel
        double c_r = random_double();
        int h = -1;
        if (c_r < r_prop) h = 0;
        else if (c_r < r_prop + g_prop) h = 1;
        else h = 2;

        //Store the probability of each channel given survival up to this point
        double r_prob_accum = r_prop;
        double g_prob_accum = g_prop;
        double b_prob_accum = b_prop;

        //Select optical thickness budget from unit Poisson
        double c_d = random_double();
        double budget = -std::log(1.0 - c_d);

        color throughput = colors::white;

        for (const medium_slice& slice : slices) {
            //Skip empty slices
            if (slice.is_empty) continue;

            const interval slice_t = slice.m_interval;
            //If we have very low density on hero channel, assume no scatter
            const double sigma_maj_h = slice.m_sigma_maj[h];

            double remaining = slice_t.size();
            double offset = 0.0;

            while (true) {
                double dt = (sigma_maj_h > epsilon) ? budget / sigma_maj_h : remaining + epsilon;

                if (dt >= remaining) {
                    // Traverse the rest of the slice
                    budget -= remaining * sigma_maj_h;

                    double r_transmittance = slice_transmittance_for_channel(slice, remaining, 0);
                    double g_transmittance = slice_transmittance_for_channel(slice, remaining, 1);
                    double b_transmittance = slice_transmittance_for_channel(slice, remaining, 2);

                    double denom = r_prob_accum * r_transmittance + g_prob_accum * g_transmittance + b_prob_accum * b_transmittance;

                    throughput *= slice_transmittance(slice, remaining);

                    if (denom) {
                        double weight = 1.0 / denom;
                        throughput *= weight;

                        r_prob_accum *= r_transmittance * weight;
                        g_prob_accum *= g_transmittance * weight;
                        b_prob_accum *= b_transmittance * weight;
                    }

                    break;
                }

                vec3 p = slice.m_local_ray.at(slice_t.min + offset + dt);

                medium_properties props = slice.sample(p);
                color actual_sigma_t = props.sigma_t;
                color actual_sigma_s = props.sigma_s;

                double r_transmittance = slice_transmittance_for_channel(slice, dt, 0);
                double g_transmittance = slice_transmittance_for_channel(slice, dt, 1);
                double b_transmittance = slice_transmittance_for_channel(slice, dt, 2);

                double acceptance = (sigma_maj_h > epsilon) ? actual_sigma_t[h] / sigma_maj_h : 0.0;

                if (random_double() < acceptance) {
                    //REAL EVENT
                    double denom =
                        actual_sigma_t[0] * r_transmittance * r_prob_accum +
                        actual_sigma_t[1] * g_transmittance * g_prob_accum +
                        actual_sigma_t[2] * b_transmittance * b_prob_accum;

                    throughput *= slice_transmittance(slice, dt);

                    if (denom > epsilon) {
                        throughput *= 1.0 / denom;
                    }

                    //DO MATERIAL SELECTION BY SIGMA S
                    double total_sigma_s_h = actual_sigma_s[h];
                    shared_ptr<medium_material> chosen_mat = nullptr;
                    double chosen_sigma_s_h = 0.0;
                    double mat_pdf = 1.0;

                    if (total_sigma_s_h > epsilon) {
                        double rand = random_double() * total_sigma_s_h;

                        double cum_w = 0.0;
                        for (const auto& mat : slice.m_mats) {
                            double w = mat->sigma_s(p)[h];
                            cum_w += w;
                            if (rand < cum_w) {
                                chosen_mat = mat;
                                chosen_sigma_s_h = w;
                                mat_pdf = chosen_sigma_s_h / total_sigma_s_h;
                                break;
                            }
                        }

                        if (!chosen_mat) {
                            chosen_mat = slice.m_mats.back();
                            chosen_sigma_s_h = chosen_mat->sigma_s(p)[h];
                            mat_pdf = chosen_sigma_s_h / total_sigma_s_h;
                        }
                    }

                    rec.m_transmittance = throughput;
                    //If we didn't choose a mat it was because scatter was < epsilon, so we absorb instead
                    rec.m_is_scatter = (chosen_mat != nullptr);
                    rec.m_mat = chosen_mat;
                    rec.m_emission = props.emission;
                    rec.set_p(p);
                    rec.set_t(slice_t.min + offset + dt);
                    rec.m_mat_pdf = mat_pdf;
                    return true;
                }
                else {
                    //NULL COLLISION
                    color sigma_n = slice.m_sigma_maj - actual_sigma_t;

                    double denom_n =
                        r_prob_accum * sigma_n[0] * r_transmittance +
                        g_prob_accum * sigma_n[1] * g_transmittance +
                        b_prob_accum * sigma_n[2] * b_transmittance;

                    throughput *= slice_transmittance(slice, dt);

                    if (denom_n > epsilon) {
                        double weight_n = sigma_n[h] / denom_n;
                        throughput *= weight_n;
                        r_prob_accum *= sigma_n[0] * r_transmittance * weight_n / sigma_n[h];
                        g_prob_accum *= sigma_n[1] * g_transmittance * weight_n / sigma_n[h];
                        b_prob_accum *= sigma_n[2] * b_transmittance * weight_n / sigma_n[h];
                    }

                    offset += dt;
                    remaining -= dt;
                    //Resample budget
                    c_d = random_double();
                    budget = -std::log(1.0 - c_d);
                }
            }
        }

        rec.m_transmittance = throughput;
        rec.m_is_scatter = false;
        rec.m_mat = nullptr;
        rec.m_mat_pdf = 1.0;
        rec.m_emission = colors::black;

        return false;
    }

    static color uninterupted_transmittance(const ray& r, medium_intersections& intersections, const interval& t) {
        std::vector<medium_slice> slices;
        intersections.get_cropped_slices(t, slices);

        color total_optical_thickness = colors::black;
        for (const medium_slice& slice : slices) {
            total_optical_thickness += slice.maj_optical_thickness;
        }

        double sum = total_optical_thickness[0] + total_optical_thickness[1] + total_optical_thickness[2];
        if (sum <= 0.0) return colors::white;

        const double uniform_prob = 1.0 / 3.0;
        const double tau_weight = 1.0 - uniform_prob;
        double r_prop = tau_weight * (total_optical_thickness[0] / sum) + uniform_prob / 3.0;
        double g_prop = tau_weight * (total_optical_thickness[1] / sum) + uniform_prob / 3.0;

        double c_r = random_double();
        int h = (c_r < r_prop) ? 0 : (c_r < r_prop + g_prop) ? 1 : 2;

        color transmittance = colors::white;
        double budget = -std::log(1.0 - random_double());

        for (const medium_slice& slice : slices) {
            if (slice.is_empty) continue;

            const interval slice_t = slice.m_interval;
            const color sigma_maj = slice.m_sigma_maj;

            double sigma_maj_h = sigma_maj[h];
            if (sigma_maj_h <= epsilon) continue;

            double remaining = slice_t.size();
            double offset = 0.0;

            while (true) {
                double dt = budget / sigma_maj_h;

                if (dt >= remaining) {
                    budget -= remaining * sigma_maj_h;
                    transmittance *= exp(-sigma_maj * remaining);
                    break;
                }

                vec3 p = slice.m_local_ray.at(slice_t.min + offset + dt);
                medium_properties props = slice.sample(p);
                color actual_sigma_t = props.sigma_t;
                color sigma_n = sigma_maj - actual_sigma_t;

                color ratio;
                for (int i = 0; i < 3; i++) {
                    ratio[i] = (sigma_maj[i] > epsilon) ? sigma_n[i] / sigma_maj[i] : 1.0;
                }
                transmittance *= exp(-sigma_maj * dt) * ratio;

                offset += dt;
                remaining -= dt;
                budget = -std::log(1.0 - random_double());
            }
        }

        return transmittance;
    }

};

#endif //BENDERER_MEDIUM_SAMPLER_H