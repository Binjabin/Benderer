//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_SURFACE_LIST_H
#define BENDERER_SURFACE_LIST_H
#include <vector>

#include "surface.h"


class surface_list : public surface {
public:
    std::vector<shared_ptr<surface>> m_surfaces;

    surface_list() {
    }

    surface_list( shared_ptr<surface> surface ) {
        add( surface );
        set_count(surface->get_count());
    }

    void clear() { m_surfaces.clear(); }

    void add( shared_ptr<surface> surface ) {
        m_surfaces.push_back( surface );
        add_hittable_properties(surface);
    }

    bool surface_hit( const ray& r, const interval& ray_t, surface_hit_rec& rec ) const override {
        surface_hit_rec temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        if (m_surfaces.empty()) return false;

        auto closest_object_i = -1;
        for (int i = 0; i < m_surfaces.size(); i++) {
            auto surface = m_surfaces[i];
            auto new_interval = interval( ray_t.min, closest_so_far );
            if ( surface->surface_hit( r, new_interval, temp_rec ) ) {
                hit_anything = true;
                closest_so_far = temp_rec.get_t();
                closest_object_i = i;
                rec = temp_rec;
            }
        }

        //Combine pdfs in list. We calculate the pdf of the sub-hittable then multiply it by the pdf of choosing this item
        if (hit_anything) {
            rec.m_pdf_v *= get_discrete_flux_pdf(closest_object_i);
        }

        return hit_anything;
    }

    bool surface_hit_check(const ray &r, interval ray_t) const override {
        if (m_surfaces.empty()) return false;

        for (int i = 0; i < m_surfaces.size(); i++) {
            auto surface = m_surfaces[i];
            if ( surface->surface_hit_check( r, ray_t ) ) {
                return true;
            }
        }

        return false;
    }

    double pdf_value(const point3& origin, const vec3& direction) const override {
        double total_flux = get_flux_weight();
        if (total_flux <= 0.0) return 0.0;

        auto sum = 0.0;
        for (const auto& surface : m_surfaces) {
            double weight = surface->get_flux_weight() / total_flux;
            sum += weight * surface->pdf_value( origin, direction );
        }

        return sum;
    }

    void compute_properties() override {
        double area = 0;
        color flux = colors::black;

        for (const auto& surface : m_surfaces) {
            surface->compute_properties();
            area += surface->get_surface_area();
            flux += surface->get_flux();
        }

        set_surface_area(area);
        set_flux(flux);
    }

    void set_explicit_light(bool is_light) override {
        for (const auto& surface : m_surfaces) {
            surface->set_explicit_light(is_light);
        }
    }

    surface_light_sample sample_light_over_flux(double seed, double running_prob) const override {
        if (m_surfaces.size() <= 0) {
            throw std::runtime_error("No objects in hittable list");
        }

        auto total_flux = get_flux_weight();
        auto sample = seed * total_flux;

        double bottom = 0;
        double top = m_surfaces[0]->get_flux_weight();
        double interval_range = top;
        int i = 0;
        while (top < sample && i + 1 < m_surfaces.size()) {
            top += m_surfaces[i+1]->get_flux_weight();
            bottom += interval_range;
            interval_range = top - bottom;
            i++;
        }

        //We end under a specific item
        double new_seed = (sample - bottom) / interval_range;
        double prob = interval_range / total_flux;
        return m_surfaces[i]->sample_light_over_flux(new_seed, running_prob * prob);

    }

    std::vector<shared_ptr<surface>> flatten() override {
        std::vector<shared_ptr<surface>> flattened;
        for (const auto& surface : m_surfaces) {
            auto child = surface ->flatten();
            flattened.insert(flattened.end(), child.begin(), child.end());
        }
        return flattened;
    }

private:
    //The probability a flux based sample selected item i out of the list
    double get_discrete_flux_pdf(int i) const {
        double total = get_flux_weight();
        double p = m_surfaces[i]->get_flux_weight() / total;
        return p;
    }
};

#endif //BENDERER_SURFACE_LIST_H