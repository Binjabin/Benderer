//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_CONSTANT_MEDIUM_H
#define BENDERER_CONSTANT_MEDIUM_H
#include "medium.h"
#include "../../../records/medium_intersection.h"
#include "../../material/medium_material.h"
#include "../../shapes/solids/solid.h"
#include <algorithm>
#include <cmath>


class primitive_medium : public medium {
public:
    primitive_medium(std::shared_ptr<solid> boundary, std::shared_ptr<medium_material> mat, int excluder_level = 0)
        : m_boundary(boundary), m_mat(mat), m_excluder_level(excluder_level) {
        set_count(1);
        set_bbox(m_boundary->bounding_box());
        set_origin(point3(0,0,0));
        set_local_furthest_point(m_boundary->furthest_point());
        set_global_furthest_point(m_boundary->furthest_point());
    }

    bool medium_hit(const ray &r, const interval& r_t, medium_intersections& rec) const override {
        if (bounding_box().hit(r, r_t) == false) return false;

        bool intersected = false;
        interval remaining = r_t;

        // Determine initial state.
        bool is_inside = m_boundary->contains(r.at(remaining.min));
        double current_t = remaining.min;

        intersection isect;
        int loop_count = 0;
        double last_hit_t = -infinity;
        while (m_boundary->intersect(r, remaining, isect)) {
            double hit_t = isect.get_t();
            if (++loop_count > 1000) {
                std::cerr << "ERROR: primitive_medium::medium_hit STUCK? loop=" << loop_count 
                          << " t=" << hit_t << " last_t=" << last_hit_t 
                          << " min=" << remaining.min << " max=" << remaining.max 
                          << " is_inside=" << is_inside << std::endl;
                break;
            }
            if (hit_t < remaining.min - epsilon) {
                 // Bug in intersect routine?
                 std::cerr << "WARNING: hit_t=" << hit_t << " < remaining.min=" << remaining.min << std::endl;
            }
            last_hit_t = hit_t;
            
            intersected = true;
            if (is_inside) {
                // We were inside, now we hit a boundary, so we exit.
                if (hit_t > current_t) {
                    rec.add(m_mat, interval(current_t, hit_t), r, m_excluder_level);
                }
                is_inside = false;
            } else {
                // We were outside, now we hit a boundary, so we enter.
                is_inside = true;
                current_t = hit_t;
            }

            double next_min = hit_t + std::max(epsilon, std::abs(hit_t) * epsilon);
            // Safety: ensure monotonic progress
            remaining.min = std::max(next_min, remaining.min + epsilon);
        }

        // If still inside at the end of the interval, add the final segment.
        if (is_inside) {
            rec.add(m_mat, interval(current_t, remaining.max), r, m_excluder_level);
            intersected = true;
        }

        return intersected;
    }

    void compute_properties() override {
        if (m_mat) {
            double volume = m_boundary->volume();
            set_volume(volume);
            set_flux(4 * pi * volume * m_mat->average_radiance());
        }
    }

    void set_explicit_light(bool is_light) override {
        m_is_explicit_light = is_light;
    }

    volume_light_sample sample_light_over_flux(double seed, double running_prob) const override {

        vec3 p = m_boundary->sample_over_volume();
        volume_light_sample result;
        result.m_radiance = m_mat->average_radiance();
        result.m_light_p = p;
        result.m_pdf_V = m_boundary->pdf_V_value(p) * running_prob;

        return result;
    }

    double pdf_value(const point3 &origin, const vec3 &direction) const override {
        double V = m_boundary->volume();
        if (V <= 0.0) return 0.0;

        vec3 d = unit_vector(direction);
        ray r(origin, d);

        double integral = 0.0;
        interval remaining = interval(0, infinity);

        bool is_inside = m_boundary->contains(r.origin());
        double current_t = 0.0;

        intersection isect;
        int loop_count = 0;
        double last_hit_t = -infinity;
        while (m_boundary->intersect(r, remaining, isect)) {
            double hit_t = isect.get_t();
            if (++loop_count > 1000) {
                std::cerr << "ERROR: primitive_medium::pdf_value STUCK? loop=" << loop_count 
                          << " t=" << hit_t << " last_t=" << last_hit_t 
                          << " min=" << remaining.min << " max=" << remaining.max 
                          << " is_inside=" << is_inside << std::endl;
                break;
            }
            last_hit_t = hit_t;

            if (is_inside) {
                if (hit_t > current_t) {
                    integral += (hit_t * hit_t * hit_t - current_t * current_t * current_t) / 3.0;
                }
                is_inside = false;
            } else {
                is_inside = true;
                current_t = hit_t;
            }

            double next_min = hit_t + std::max(epsilon, std::abs(hit_t) * epsilon);
            remaining.min = std::max(next_min, remaining.min + epsilon);
        }

        return integral / V;
    }

    std::vector<shared_ptr<medium>> flatten() override {
        std::vector<shared_ptr<medium>> flattened;
        flattened.push_back(shared_from_this());
        return flattened;
    }
    
private:
    shared_ptr<solid> m_boundary;
    shared_ptr<medium_material> m_mat;
    //Excludes any volumes in the region of lower level
    int m_excluder_level;
    bool m_is_explicit_light = false;
};


#endif //BENDERER_CONSTANT_MEDIUM_H