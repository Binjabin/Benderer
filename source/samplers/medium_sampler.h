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
        return exp(-slice.optical_thickness);
    }

    static color slice_transmittance(const medium_slice& slice, double t) {
        return exp(-slice.sigma_t * t);
    }

    static double slice_transmittance_for_channel(const medium_slice& slice, int channel) {
        return std::exp(-slice.optical_thickness[channel]);
    }

    static double slice_transmittance_for_channel(const medium_slice& slice, double t, int channel) {
        return std::exp(-slice.sigma_t[channel] * t);
    }

    static bool sample_distance(const ray& r, medium_intersections& intersections, const interval& t, medium_hit_rec& rec) {

        std::vector<medium_slice> slices;
        intersections.get_cropped_slices(t, slices);

        //Calculate total optical thickness
        color total_optical_thickness = colors::black;
        for (const medium_slice& slice : slices) {
            total_optical_thickness += slice.optical_thickness;
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

        double r_prop = total_optical_thickness[0] / sum;
        double g_prop = total_optical_thickness[1] / sum;
        double b_prop = total_optical_thickness[2] / sum;

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
        double budget = -std::log(1 - c_d);

        color throughput = colors::white;

        for (const medium_slice& slice : slices) {
            //Skip empty slices
            if (slice.is_empty) continue;

            const interval slice_t = slice.m_interval;
            const double d_t = slice_t.size();
            double sigma_t_h = slice.sigma_t[h];

            //If we have very low density on hero channel, assume no scatter
            double t = d_t + epsilon;
            if (sigma_t_h > epsilon) {
                t = budget / sigma_t_h;
            }

            if (t > d_t) {
                //Have traversed whole segment
                budget -= d_t * sigma_t_h;
                throughput *= slice_transmittance(slice);

                double r_transmittance = slice_transmittance_for_channel(slice, 0);
                double g_transmittance = slice_transmittance_for_channel(slice, 1);
                double b_transmittance = slice_transmittance_for_channel(slice, 2);

                double weight = 1.0 / (r_prob_accum * r_transmittance + g_prob_accum * g_transmittance + b_prob_accum * b_transmittance);

                throughput *= (weight);

                //update probabilities
                r_prob_accum *= slice_transmittance_for_channel(slice, 0) * weight;
                g_prob_accum *= slice_transmittance_for_channel(slice, 1) * weight;
                b_prob_accum *= slice_transmittance_for_channel(slice, 2) * weight;
            }
            else {
                //Scatter event!
                vec3 p = r.at(slice_t.min + t);

                double total_sigma_s = 0.0;
                for (const auto& mat : slice.m_mats) {
                    total_sigma_s += mat->sigma_s(p)[h];
                }

                shared_ptr<medium_material> chosen_mat = nullptr;
                double chosen_sigma_s_h = 0.0;
                double mat_pdf = 1.0;

                if (total_sigma_s > epsilon) {
                    double rand = random_double() * total_sigma_s;

                    double cum_w = 0.0;
                    for (const auto& mat : slice.m_mats) {
                        double w = mat->sigma_s(p)[h];
                        cum_w += w;
                        if (rand < cum_w) {
                            chosen_mat = mat;
                            chosen_sigma_s_h = w;
                            mat_pdf = chosen_sigma_s_h / total_sigma_s;
                            break;
                        }
                    }

                    if (!chosen_mat) {
                        chosen_mat = slice.m_mats.back();
                        chosen_sigma_s_h = chosen_mat->sigma_s(p)[h];
                        mat_pdf = chosen_sigma_s_h / total_sigma_s;
                    }
                }

                throughput *= slice_transmittance(slice, t);

                double r_transmittance = slice_transmittance_for_channel(slice, t, 0);
                double g_transmittance = slice_transmittance_for_channel(slice, t, 1);
                double b_transmittance = slice_transmittance_for_channel(slice, t, 2);

                double weight = 1.0 / (
                    slice.sigma_t[0] * r_transmittance * r_prob_accum +
                    slice.sigma_t[1] * g_transmittance * g_prob_accum +
                    slice.sigma_t[2] * b_transmittance * b_prob_accum
                );

                throughput *= weight;

                rec.m_transmittance = throughput;
                rec.m_is_scatter = (chosen_mat != nullptr);
                rec.m_mat = chosen_mat;
                rec.m_emission = slice.emission;
                rec.set_p(p);
                rec.set_t(slice_t.min + t);
                rec.m_mat_pdf = mat_pdf;

                return true;
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

        //Calculate total optical thickness
        color total_optical_thickness = colors::black;
        for (const medium_slice& slice : slices) {
            total_optical_thickness += slice.optical_thickness;
        }

        return exp(-total_optical_thickness);
    }

};

#endif //BENDERER_MEDIUM_SAMPLER_H