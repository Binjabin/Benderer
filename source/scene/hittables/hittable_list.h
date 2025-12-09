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

    bool hit( const ray& r, interval ray_t, hit_record& rec ) const override {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for ( const auto& object : objects ) {
            //only check intersections closer than current furthest
            auto new_interval = interval( ray_t.min, closest_so_far );
            if ( object->hit( r, new_interval, temp_rec ) ) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
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

private:
    aabb bbox;
};


#endif //HITTABLE_LIST_H
