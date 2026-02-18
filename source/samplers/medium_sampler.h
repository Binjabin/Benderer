//
// Created by binjabin on 1/30/26.
//

#ifndef BENDERER_MEDIUM_SAMPLER_H
#define BENDERER_MEDIUM_SAMPLER_H
#include <memory>

#include "../scene/material/medium_material.h"
#include "../records/medium_intersection.h"


class medium_sampler {
public:
    static inline double sigma_scalar(const color& c) {
        return max_component(c);
    }

    static bool sample_distance(const ray& r, medium_intersections& intersections, interval t, medium_hit_rec& rec) {

        auto slices = intersections.get_cropped_slices(t);

        color transmittance = colors::white;
        double transmittance_pdf_scalar = 1.0;

        for (const medium_slice& slice : slices) {
            //Skip empty slices

            const auto& mats = slice.m_mats;
            if (mats.empty()) continue;


            interval w_t = slice.m_interval;
            double t0 = w_t.min;
            double t1 = w_t.max;

            double cursor_t = t0;
            point3 p = r.at(cursor_t);

            //Per-Slice co-efficients
            color sigma_t_color_total = colors::black;
            double sigma_t_scalar_total = 0.0;
            double sigma_maj_scalar_total = 0.0;

            for (const shared_ptr<medium_material>& mat : mats) {
                const color sigma_t_i = mat->sigma_t(p);
                sigma_t_color_total += sigma_t_i;
                sigma_t_scalar_total += sigma_scalar(sigma_t_i);
                sigma_maj_scalar_total += sigma_scalar(mat->sigma_maj());
            }

            //No extinction in this slice: Skip
            if (sigma_t_scalar_total <= 0.0 || sigma_maj_scalar_total <= 0.0) {
                // No extinction in this slice; nothing happens, just continue
                continue;
            }

            while (true) {
                //Sample free-flight distance
                double u = random_double();
                double dt = (-std::log(1 - u)) / sigma_maj_scalar_total;

                if (cursor_t + dt > t1) {
                    //If we exit slice, advance to slice end and continue
                    const double dist = t1 - cursor_t;
                    transmittance *= exp(-sigma_t_color_total * dist);
                    transmittance_pdf_scalar *= std::exp(-sigma_t_scalar_total * dist);
                    break;
                }

                //Otherwise we carry on in slice until where our event occurs!
                cursor_t += dt;
                p = r.at(cursor_t);
                transmittance *= exp(-sigma_t_color_total * dt);
                transmittance_pdf_scalar *= std::exp(-sigma_t_scalar_total * dt);

                //What sort of event?
                const double u2 = random_double();
                const double accept_prob = sigma_t_scalar_total / sigma_maj_scalar_total;

                //If we pass this check we have a null collision, keep marching
                if (u2 >= accept_prob) continue;

                //Otherwise we have a REAL event. Choose the medium that caused it
                std::vector<double> sigma_t_weights;
                sigma_t_weights.reserve(mats.size());
                double sigma_t_sum = 0.0;
                for (const auto& mat : mats) {
                    const double w = sigma_scalar(mat->sigma_t(p));
                    sigma_t_weights.push_back(w);
                    sigma_t_sum += w;
                }
                int chosen = 0;
                if (sigma_t_sum > 0.0) {
                    double pick = random_double() * sigma_t_sum;
                    while (pick > sigma_t_weights[chosen]) {
                        pick -= sigma_t_weights[chosen];
                        ++chosen;
                    }
                }
                shared_ptr<medium_material> chosen_mat = mats[chosen];

                //Choose scatter vs absorb for the chosen medium
                const double sigma_t_chosen_scalar = sigma_scalar(chosen_mat->sigma_t(p));
                const double sigma_s_chosen_scalar = sigma_scalar(chosen_mat->sigma_s(p));
                const double p_scatter = (sigma_t_chosen_scalar > 0.0) ? (sigma_s_chosen_scalar / sigma_t_chosen_scalar) : 0.0;

                //Populate record
                rec.set_t(cursor_t);
                rec.set_p(p);
                rec.m_transmittance = transmittance;
                rec.m_transmittance_pdf_scalar = transmittance_pdf_scalar;
                rec.m_mat = chosen_mat;

                const double u3 = random_double();
                if (u3 < p_scatter) {
                    rec.m_is_scatter = true;
                    rec.m_sigma_s = chosen_mat->sigma_s(p);
                    rec.m_sigma_s_scalar = sigma_scalar(rec.m_sigma_s);
                }
                else {
                    rec.m_is_scatter = false;
                }

                return true;
            }
        }

        //No event at all!

        rec.m_transmittance = transmittance;
        rec.m_transmittance_pdf_scalar = transmittance_pdf_scalar;
        rec.m_is_scatter = false;
        rec.m_mat = nullptr;

        return false;
    }

    //Calculate (Homogeneous) Transmittance along the ray
    static color transmittance_homogeneous(const ray& r, medium_intersections& intersections, interval t) {
        auto slices = intersections.get_cropped_slices(t);
        color transmission = colors::white;

        for (const medium_slice& slice : slices) {

            //Skip vacuums
            if (slice.m_mats.empty()) continue;

            interval w_t = slice.m_interval;
            double dt = w_t.size();

            //Since we are homogeneous, this is fine
            point3 p = r.at(w_t.min);

            color sigma_t_sum = colors::black;
            for (auto& mat : slice.m_mats) {
                sigma_t_sum += mat->sigma_t(p);
            }

            //If sigma_t is 0, transmittance is always 1!
            if (sigma_scalar(sigma_t_sum) <= 0.0) continue;

            //Calculate the beer-lambert based on this per-slice
            color local_transmission = exp(-sigma_t_sum * dt);
            transmission = transmission * local_transmission;
        }
        
        return transmission;
    }
};

#endif //BENDERER_MEDIUM_SAMPLER_H