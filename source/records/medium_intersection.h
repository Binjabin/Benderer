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
    medium_intersection(shared_ptr<medium_material> mat, interval t, int priority)
        : m_mat(std::move(mat)), m_t(t), m_excluder_priority(priority) {}

    shared_ptr<medium_material> m_mat;
    interval m_t;
    int m_excluder_priority;
};

//A sub-segment of a ray with some media on it
class medium_slice {
public:
    std::vector<shared_ptr<medium_material>> m_mats;
    interval m_interval;

    medium_slice(std::vector<shared_ptr<medium_material>> ms, interval t)
        : m_mats(std::move(ms)), m_interval(t) {

        sigma_t = colors::black;
        sigma_s = colors::black;
        emission = colors::black;
        for (const auto& mat : m_mats) {
            //Arbitrary location since volumes are homogeneous!
            sigma_t += mat->sigma_t(vec3(0.0, 0.0, 0.0));
            sigma_s += mat->sigma_s(vec3(0.0, 0.0, 0.0));
            emission += mat->emission(vec3(0.0, 0.0, 0.0));
        }
        optical_thickness = sigma_t * m_interval.size();

        is_empty = m_mats.empty();
    }

    color sigma_s;
    color sigma_t;
    color emission;
    color optical_thickness;
    bool is_empty;
};

class medium_intersections {
public:
    medium_intersections() = default;

    void add(shared_ptr<medium_material> mat, interval med_t, int excluder_priority) {
        m_intersections.emplace_back(std::move(mat), med_t, excluder_priority);
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
                out_slices.push_back({slice.m_mats, interval(new_min, new_max)});
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
                    std::vector<shared_ptr<medium_material>> filtered_mats;
                    for (const auto* rec : m_active) {
                        if (rec->m_excluder_priority == max_priority && rec->m_mat) {
                            filtered_mats.push_back(rec->m_mat);
                        }
                    }

                    //Add all the unfiltered lists and intervals.
                    if (!filtered_mats.empty()) {
                        m_slices.push_back({std::move(filtered_mats), interval(start_t, end_t)});
                    }
                }
            }
        }
        sliced = true;
    }

};

#endif //BENDERER_MEDIUM_HIT_RECORD_H