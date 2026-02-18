//
// Created by binjabin on 1/18/26.
//

#ifndef BENDERER_MEDIUM_HIT_RECORD_H
#define BENDERER_MEDIUM_HIT_RECORD_H
#include <algorithm>
#include <memory>
#include "interaction.h"

//tell compiler we handle what this is later
class medium_material;

class medium_intersection {
public:
    ~medium_intersection() = default;
    medium_intersection(shared_ptr<medium_material> mat, interval med_t)
        : m_mat(mat){
        m_interval_t = med_t;
    }

    shared_ptr<medium_material> m_mat;
    interval m_interval_t;
};

struct medium_boundary_event {
    shared_ptr<medium_material> m_mat;
    double m_t;
    bool m_enter;
};

struct medium_slice {
    std::vector<shared_ptr<medium_material>> m_mats;
    interval m_interval;
};

class medium_intersections {
public:
    medium_intersections(){
    }

    void add(shared_ptr<medium_material> mat, interval med_t) {
        auto new_rec = medium_intersection(mat, med_t);
        m_recs.push_back(new_rec);
        sliced = false;
    }

    void fuse(const medium_intersections& other) {
        m_recs.reserve(m_recs.size() + other.m_recs.size());
        m_recs.insert(m_recs.end(), other.m_recs.begin(), other.m_recs.end());
        sliced = false;
    }

    const std::vector<medium_slice>& slices() {
        if (!sliced) {
            make_slices();
        }
        return m_slices;
    }

    std::vector<medium_slice> get_cropped_slices(interval t) {
        if (!sliced) {
            make_slices();
        }

        std::vector<medium_slice> result;

        if (m_slices.empty()) return result;

        int count = m_slices.size();

        int i = -1;
        double current_slice_end_t = -infinity;

        for (medium_slice slice : m_slices) {
            interval slice_t = slice.m_interval;
            double new_min = slice_t.min;
            double new_max = slice_t.max;

            if (slice_t.max < t.min || slice_t.min > t.max) {
                //Slice isn't in region, skip
                continue;
            }
            // Clamp slice to requested interval: [max(slice.min, t.min), min(slice.max, t.max)]
            new_min = (new_min < t.min) ? t.min : new_min;
            new_max = (new_max > t.max) ? t.max : new_max;

            interval new_interval = interval(new_min, new_max);
            medium_slice new_slice = slice;
            new_slice.m_interval = new_interval;
            result.push_back(new_slice);
        }

        return result;
    }

    std::vector<medium_intersection> m_recs;

private:
    bool sliced = false;
    std::vector<medium_slice> m_slices;

    void make_slices() {
        m_slices = std::vector<medium_slice>();

        std::vector<medium_boundary_event> events;
        events.reserve(m_recs.size() * 2);

        for (const medium_intersection& rec : m_recs) {
            auto in = medium_boundary_event(rec.m_mat, rec.m_interval_t.min, true);
            auto out = medium_boundary_event(rec.m_mat, rec.m_interval_t.max, false);
            events.push_back(in);
            events.push_back(out);
        }

        std::sort(events.begin(), events.end(),
        [](const medium_boundary_event& a, const medium_boundary_event& b) {
            if (a.m_t != b.m_t) { return a.m_t < b.m_t; } ;
            return a.m_enter && !b.m_enter; ;
        });

        std::vector<std::shared_ptr<medium_material>> active;

        int i = 0;
        double start_t = -infinity;
        double end_t = -infinity;

        while (i < events.size()) {
            int j = i;

            start_t = events[i].m_t;

            while (j < events.size() && events[j].m_t <= start_t + epsilon) {
                //Consider this the same t
                auto e = events[j];

                if (e.m_enter) {
                    active.push_back(e.m_mat);
                }
                else {
                    auto it = std::find(active.begin(), active.end(), e.m_mat);
                    active.erase(it);
                }

                j++;
            }

            if (j < events.size()) {
                end_t = events[j].m_t;
            }
            else {
                end_t = infinity;
            }
            auto slice = medium_slice(active, interval(start_t, end_t));

            m_slices.push_back(slice);

            start_t = end_t;
            i = j;
        }

        sliced = true;
    }


};

#endif //BENDERER_MEDIUM_HIT_RECORD_H