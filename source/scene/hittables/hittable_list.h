//
// Created by binjabin on 6/24/25.
//

#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"

#include <vector>


class hittable_list : public hittable {
public:
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {
    }

    hittable_list( shared_ptr<hittable> object ) {
        add( object );
        set_count(object->get_count());
        set_surface_area(object->get_surface_area());
        set_flux_rgb(object->get_flux_rgb());
    }

    void clear() { objects.clear(); }

    void add( shared_ptr<hittable> object ) {
        objects.push_back( object );
        bbox = aabb( bbox, object->bounding_box() );
        set_count(get_count() + object->get_count());
        set_surface_area(get_surface_area() + object->get_surface_area());
        set_flux_rgb(get_flux_rgb() + object->get_flux_rgb());
    }

    bool hit( const ray& r, interval ray_t, surface_hit& rec ) const override {
        surface_hit temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        if (objects.empty()) return false;

        auto closest_object_i = -1;
        for (int i = 0; i < objects.size(); i++) {
            auto object = objects[i];
            auto new_interval = interval( ray_t.min, closest_so_far );
            if ( object->hit( r, new_interval, temp_rec ) ) {
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

    aabb bounding_box() const override { return bbox; }

    double pdf_value(const point3& origin, const vec3& direction) const override {
        auto weight = 1.0 / objects.size();
        auto sum = 0.0;

        for (const auto& object : objects) {
            sum += weight * object->pdf_value( origin, direction );
        }

        return sum;
    }

    vec3 random(const point3& origin) const override {
        auto int_size = int(objects.size());
        auto obj = objects[random_int(0, int_size-1)];
        return obj->random(origin);
    }

    void compute_properties() override {

        int sum_count = 0;
        double sum_area = 0;
        vec3 sum_flux_rgb = vec3(0,0,0);

        for (const auto& object : objects) {
            object->compute_properties();
            sum_count += object->get_count();
            sum_area += object->get_surface_area();
            sum_flux_rgb += object->get_flux_rgb();
        }

        set_count(sum_count);
        set_surface_area(sum_area);
        set_flux_rgb(sum_flux_rgb);
    }

    void set_explicit_light(bool is_light) override {
        for (const auto& object : objects) {
            object->set_explicit_light(is_light);
        }
    }

    surface_light_sample sample_light_over_flux(double seed, double running_prob) const override {
        if (objects.size() <= 0) {
            throw std::runtime_error("No objects in hittable list");
        }

        auto total_flux = get_flux_weight();
        auto sample = seed * total_flux;

        double bottom = 0;
        double top = objects[0]->get_flux_weight();
        double interval_range = top;
        int i = 0;
        while (top < sample && i + 1 < objects.size()) {
            top += objects[i+1]->get_flux_weight();
            bottom += interval_range;
            interval_range = top - bottom;
        }

        //We end under a specific item
        double new_seed = (sample - bottom) / interval_range;
        double prob = interval_range / total_flux;
        return objects[i]->sample_light_over_flux(new_seed, running_prob * prob);

    }

private:
    //The probability a flux based sample selected item i out of the list
    double get_discrete_flux_pdf(int i) const {
        double total = get_flux_weight();
        double p = objects[i]->get_flux_weight() / total;
        return p;
    }

    aabb bbox;
};


#endif //HITTABLE_LIST_H
