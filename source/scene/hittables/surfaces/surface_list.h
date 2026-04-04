//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_SURFACE_LIST_H
#define BENDERER_SURFACE_LIST_H
#include <vector>

#include "surface.h"


class surface_list : public surface {
public:
    std::vector<shared_ptr<surface>> surfaces;

    surface_list() {
    }

    surface_list( shared_ptr<surface> surface ) {
        add( surface );
        set_count(surface->get_count());
    }

    void clear() { surfaces.clear(); }

    void add( shared_ptr<surface> surface ) {
        surfaces.push_back( surface );
        bbox = aabb( bbox, surface->bounding_box() );

        if (get_count() == 0) {
            m_origin = surface->origin();
            m_local_furthest_point = surface->local_furthest_point();
        }
        else {
            vec3 offset = surface->origin() - m_origin;
            double dist = offset.length();
            double new_rad = (1.0 / 2.0) * (dist + m_local_furthest_point + surface->local_furthest_point());
            m_origin = m_origin + (1.0 / dist) * (new_rad - m_local_furthest_point) * offset;
            m_local_furthest_point = new_rad;
        }

        m_global_furthest_point = std::max(m_global_furthest_point, surface->global_furthest_point());
        set_count(get_count() + surface->get_count());
    }

    bool surface_hit( const ray& r, interval ray_t, surface_hit_rec& rec ) const override {
        surface_hit_rec temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        if (surfaces.empty()) return false;

        auto closest_object_i = -1;
        for (int i = 0; i < surfaces.size(); i++) {
            auto surface = surfaces[i];
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
        if (surfaces.empty()) return false;

        for (int i = 0; i < surfaces.size(); i++) {
            auto surface = surfaces[i];
            if ( surface->surface_hit_check( r, ray_t ) ) {
                return true;
            }
        }

        return false;
    }

    double pdf_value(const point3& origin, const vec3& direction) const override {
        auto weight = 1.0 / surfaces.size();
        auto sum = 0.0;

        for (const auto& object : surfaces) {
            sum += weight * object->pdf_value( origin, direction );
        }

        return sum;
    }

    /*
    vec3 random(const point3& origin) const override {
        auto int_size = int(surfaces.size());
        auto obj = surfaces[random_int(0, int_size-1)];
        return obj->random(origin);
    }
    */

    void compute_properties() override {

        int sum_count = 0;
        double sum_area = 0;
        vec3 sum_flux_rgb = vec3(0,0,0);

        for (const auto& surface : surfaces) {
            surface->compute_properties();
            sum_count += surface->get_count();
            sum_area += surface->get_surface_area();
            sum_flux_rgb += surface->get_flux_rgb();
        }

        set_count(sum_count);
        set_surface_area(sum_area);
        set_flux_rgb(sum_flux_rgb);
    }

    void set_explicit_light(bool is_light) override {
        for (const auto& surface : surfaces) {
            surface->set_explicit_light(is_light);
        }
    }

    surface_light_sample sample_light_over_flux(double seed, double running_prob) const override {
        if (surfaces.size() <= 0) {
            throw std::runtime_error("No objects in hittable list");
        }

        auto total_flux = get_flux_weight();
        auto sample = seed * total_flux;

        double bottom = 0;
        double top = surfaces[0]->get_flux_weight();
        double interval_range = top;
        int i = 0;
        while (top < sample && i + 1 < surfaces.size()) {
            top += surfaces[i+1]->get_flux_weight();
            bottom += interval_range;
            interval_range = top - bottom;
            i++;
        }

        //We end under a specific item
        double new_seed = (sample - bottom) / interval_range;
        double prob = interval_range / total_flux;
        return surfaces[i]->sample_light_over_flux(new_seed, running_prob * prob);

    }

    aabb bounding_box() const override { return bbox; }

    double global_furthest_point() const override {
        return m_global_furthest_point;
    }

    double local_furthest_point() const override {
        return m_local_furthest_point;
    }

    vec3 origin() const override {
        return m_origin;
    }

    std::vector<shared_ptr<surface>> flatten() const override {
        std::vector<shared_ptr<surface>> flattened;
        for (const auto& surface : surfaces) {
            auto child = surface ->flatten();
            flattened.insert(flattened.end(), child.begin(), child.end());
        }
        return flattened;
    }

private:
    //The probability a flux based sample selected item i out of the list
    double get_discrete_flux_pdf(int i) const {
        double total = get_flux_weight();
        double p = surfaces[i]->get_flux_weight() / total;
        return p;
    }

    aabb bbox;

    point3 m_origin;
    double m_local_furthest_point;
    double m_global_furthest_point;
};

#endif //BENDERER_SURFACE_LIST_H