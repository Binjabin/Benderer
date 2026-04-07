//
// Created by binjabin on 1/18/26.
//

#ifndef BENDERER_MEDIUM_HIT_RECORD_H
#define BENDERER_MEDIUM_HIT_RECORD_H
#include <algorithm>
#include <memory>
#include "interaction.h"

#include "../scene/material/medium_material.h"

//The intersection along a ray with a medium
struct medium_intersection {
    medium_intersection(shared_ptr<medium_material> mat, interval t, const ray& local_ray, int priority)
        : m_mat(std::move(mat)), m_t(t), m_local_ray(local_ray), m_excluder_priority(priority) {}

    shared_ptr<medium_material> m_mat;
    interval m_t;
    ray m_local_ray;
    int m_excluder_priority;
};

struct slice_entry {
    shared_ptr<medium_material> m_mat;
    ray m_local_ray;
};

//A sub-segment of a ray with some media on it
class medium_slice {
public:
    std::vector<slice_entry> m_entries;
    interval m_interval;

    medium_slice(std::vector<slice_entry> entries, interval t)
        : m_entries(std::move(entries)), m_interval(t) {

        m_sigma_maj = colors::black;
        for (const auto& entry : m_entries) {
            //Arbitrary location since volumes are homogeneous!
            m_sigma_maj += entry.m_mat->sigma_maj();
        }
        maj_optical_thickness = m_sigma_maj * m_interval.size();

        is_empty = m_entries.empty();
    }

    color sigma_s(const double t) const {
        color sigma_s = colors::black;
        for (const auto& entry : m_entries) {
            vec3 local_p = entry.m_local_ray.at(t);
            sigma_s += entry.m_mat->sigma_s(local_p);
        }
        return sigma_s;
    }

    color sigma_t(const double t) const {
        color sigma_t = colors::black;
        for (const auto& entry : m_entries) {
            vec3 local_p = entry.m_local_ray.at(t);
            sigma_t += entry.m_mat->sigma_t(local_p);
        }
        return sigma_t;
    }

    color emission(const double t) const {
        color emission = colors::black;
        for (const auto& entry : m_entries) {
            vec3 local_p = entry.m_local_ray.at(t);
            emission += entry.m_mat->emission(local_p);
        }
        return emission;
    }

    medium_properties sample(const double t) const {
        medium_properties total = {colors::black, colors::black, colors::black};
        for (const auto& entry : m_entries) {
            vec3 local_p = entry.m_local_ray.at(t);
            medium_properties props = entry.m_mat->sample(local_p);
            total.sigma_t += props.sigma_t;
            total.sigma_s += props.sigma_s;
            total.emission += props.emission;
        }
        return total;
    }

    color m_sigma_maj;
    color maj_optical_thickness;
    bool is_empty;
};

class medium_intersections {
public:
    medium_intersections() = default;

    void add(shared_ptr<medium_material> mat, interval med_t, const ray& local_ray, int excluder_priority = 0) {
        m_intersections.emplace_back(std::move(mat), med_t, local_ray, excluder_priority);
        sliced = false;
    }

    void fuse(const medium_intersections& other) {
        if (other.m_intersections.empty()) return;
        m_intersections.insert(m_intersections.end(), other.m_intersections.begin(), other.m_intersections.end());
        sliced = false;
    }

    const std::vector<medium_slice>& slices() {
        if (!sliced) {
            make_slices();
        }
        return m_slices;
    }

    void get_cropped_slices(interval t, std::vector<medium_slice>& out_slices){
        //Make the slices if we haven't already.
        if (!sliced) {
            make_slices();
        }

        out_slices.clear();

        for (const auto& slice : m_slices) {
            if (slice.m_interval.max <= t.min || slice.m_interval.min >= t.max) continue;

            double new_min = std::max(slice.m_interval.min, t.min);
            double new_max = std::min(slice.m_interval.max, t.max);

            if (new_max > new_min) {
                out_slices.push_back({slice.m_entries, interval(new_min, new_max)});
            }
        }
    }

    std::vector<medium_intersection> m_intersections;

private:
    bool sliced = false;
    std::vector<medium_slice> m_slices;

    struct event {
        double m_t;
        bool m_enter;
        const medium_intersection* m_rec;

        bool operator<(const event& other) const {
            if (std::abs(m_t - other.m_t) > epsilon)
                return m_t < other.m_t;
            //We take enters before exits
            return m_enter > other.m_enter;
        }
    };

    std::vector<event> m_events;
    std::vector<const medium_intersection*> m_active;

    void make_slices() {
        m_slices.clear();
        if (m_intersections.empty()) {
            sliced = true;
            return;
        }

        //Convert the intersections into ordered slices
        m_events.clear();
        for (const auto& rec : m_intersections) {
            m_events.push_back({rec.m_t.min, true, &rec});
            m_events.push_back({rec.m_t.max, false, &rec});
        }
        std::sort(m_events.begin(), m_events.end());

        //Go through the events tracking active media
        m_active.clear();
        const int event_count = m_events.size();

        for (size_t i = 0; i < event_count; ) {
            double start_t = m_events[i].m_t;

            // Process all events at current t
            while (i < event_count && (std::abs(m_events[i].m_t - start_t) <= epsilon)) {
                const event& ev = m_events[i];
                if (ev.m_enter) {
                    //Add to active media
                    m_active.push_back(ev.m_rec);
                }
                else {
                    //Find and remove the active media
                    auto it = std::find(m_active.begin(), m_active.end(), ev.m_rec);
                    //Order doesn't matter so we can swap + pop
                    if (it != m_active.end()) {
                        std::swap(*it, m_active.back());
                        m_active.pop_back();
                    }
                }
                i++;
            }

            //If we have items and a valid interval
            if (i < event_count && !m_active.empty()) {
                //End interval at start of next one
                double end_t = m_events[i].m_t;

                if (end_t > start_t + epsilon) {
                    //Find highest priority in intersection list
                    int max_priority = std::numeric_limits<int>::min();
                    for (const auto* rec : m_active) {
                        if (rec->m_excluder_priority > max_priority) {
                            max_priority = rec->m_excluder_priority;
                        }
                    }

                    //choose everything of equal priority
                    std::vector<slice_entry> filtered_slices;
                    for (const auto* rec : m_active) {
                        if (rec->m_excluder_priority == max_priority && rec->m_mat) {
                            slice_entry entry{rec->m_mat, rec->m_local_ray};
                            filtered_slices.push_back(entry);
                        }
                    }

                    //Add all the unfiltered lists and intervals.
                    if (!filtered_slices.empty()) {
                        m_slices.push_back({std::move(filtered_slices), interval(start_t, end_t)});
                    }
                }
            }
        }
        sliced = true;
    }

};

#endif //BENDERER_MEDIUM_HIT_RECORD_H