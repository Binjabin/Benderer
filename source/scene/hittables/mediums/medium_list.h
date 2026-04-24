//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_MEDIUM_LIST_H
#define BENDERER_MEDIUM_LIST_H

//
// Created by binjabin on 1/23/26.
//

#include <vector>
#include "medium.h"


class medium_list : public medium {
public:
    std::vector<shared_ptr<medium>> m_mediums;

    medium_list() {
    }

    medium_list( shared_ptr<medium> mediums ) {
        add( mediums );
        set_count(mediums->get_count());
    }

    void clear() { m_mediums.clear(); }

    void add( shared_ptr<medium> medium ) {
        m_mediums.push_back( medium );
        add_hittable_properties(medium);
    }

    bool medium_hit( const ray& r, const interval& ray_t, medium_intersections& rec ) const override {
        if (m_mediums.empty()) return false;

        bool hit_anything = false;

        for (int i = 0; i < m_mediums.size(); i++) {
            auto medium = m_mediums[i];
            medium_intersections sub_recs;
            if ( medium->medium_hit( r, ray_t, sub_recs ) ) {
                hit_anything = true;
                rec.fuse(sub_recs);
            }
        }

        return hit_anything;
    }

    double pdf_value(const point3& origin, const vec3& direction) const override {
        double total_flux = get_flux_weight();
        if (total_flux <= 0.0) return 0.0;

        auto sum = 0.0;
        for (const auto& medium : m_mediums) {
            double weight = medium->get_flux_weight() / total_flux;
            sum += weight * medium->pdf_value( origin, direction );
        }
        return sum;
    }

    void compute_properties() override {

        int sum_count = 0;
        double sum_volume = 0;
        vec3 sum_flux_rgb = vec3(0,0,0);

        for (const auto& medium : m_mediums) {
            medium->compute_properties();
            sum_count += medium->get_count();
            sum_volume += medium->get_volume();
            sum_flux_rgb += medium->get_flux();
        }

        set_count(sum_count);
        set_volume(sum_volume);
        set_flux(sum_flux_rgb);
    }

    void set_explicit_light(bool is_light) override {
        for (const auto& medium : m_mediums) {
            medium->set_explicit_light(is_light);
        }
    }

    volume_light_sample sample_light_over_flux(double seed, double running_prob) const override {
        if (m_mediums.size() <= 0) {
            throw std::runtime_error("No objects in hittable list");
        }

        auto total_flux = get_flux_weight();
        auto sample = seed * total_flux;

        double bottom = 0;
        double top = m_mediums[0]->get_flux_weight();
        double interval_range = top;
        int i = 0;
        while (top < sample && i + 1 < m_mediums.size()) {
            top += m_mediums[i+1]->get_flux_weight();
            bottom += interval_range;
            interval_range = top - bottom;
            i++;
        }

        //We end under a specific item
        double new_seed = (sample - bottom) / interval_range;
        double prob = interval_range / total_flux;
        return m_mediums[i]->sample_light_over_flux(new_seed, running_prob * prob);

    }


    std::vector<shared_ptr<medium>> flatten() override {
        std::vector<shared_ptr<medium>> flattened;
        for (const auto& medium : m_mediums) {
            auto child = medium ->flatten();
            flattened.insert(flattened.end(), child.begin(), child.end());
        }
        return flattened;
    }

};

#endif //BENDERER_MEDIUM_LIST_H