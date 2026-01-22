//
// Created by binjabin on 7/8/25.
//

#ifndef BVH_H
#define BVH_H

#include "hittable.h"
#include "hittable_list.h"

#include <algorithm> //for sort


//acceleration structure for handling lots of objects in sub-linear time
class bvh_node : public hittable {
public:
    bvh_node( hittable_list list ) : bvh_node( list.objects, 0, list.objects.size() ) {
    }

    //construct leaf node. list of hittable objects
    bvh_node( std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end ) {
        //create a bounding box of all source items
        bbox = aabb::empty;
        for ( size_t object_index = start; object_index < end; object_index++ ) {
            bbox = aabb(bbox, objects[object_index]->bounding_box());
        }

        //select random axis to sort along, and get function for sorting
        int axis = bbox.longest_axis();
        auto comparator = ( axis == 0 ) ? box_x_compare : ( ( axis == 1 ) ? box_y_compare : box_z_compare );
        size_t object_span = end - start;

        //if there are 1 or 2 objects, just split them the obvious way, and make the subtrees just the objects
        if ( object_span == 1 ) {
            left = right = objects[start];
        }
        else if ( object_span == 2 ) {
            left = objects[start];
            right = objects[start + 1];
        }
        else {
            //sort along some axis (from comparator)
            std::sort( std::begin( objects ) + start, std::begin( objects ) + end, comparator );
            //then split in through the middle into left and right subtrees
            auto mid = start + object_span / 2;
            left = make_shared<bvh_node>( objects, start, mid );
            right = make_shared<bvh_node>( objects, mid, end );
        }

        int count_sum = 0;
        double area_sum = 0;
        vec3 flux_sum = vec3(0,0,0);
        for ( size_t i = start; i < end; i++ ) {
            count_sum += objects[i]->get_count();
            area_sum += objects[i]->get_surface_area();
            flux_sum += objects[i]->get_flux_rgb();
        }
        set_count(count_sum);
        set_surface_area(area_sum);
        set_flux_rgb(flux_sum);
    }

    //check if we hit any objects in the subtree
    bool hit( const ray& r, interval ray_t, surface_hit& rec ) const override {
        if ( !bbox.hit( r, ray_t ) )
            return false;

        surface_hit l_rec = surface_hit();
        bool hit_left = left->hit( r, ray_t, l_rec );

        surface_hit r_rec = surface_hit();
        interval right_ray_t = interval( ray_t.min, hit_left ? l_rec.get_t() : ray_t.max );
        bool hit_right = right->hit( r, right_ray_t, r_rec );

        if (!hit_left && !hit_right) {
            return false;
        }

        //Calculate the probability of choosing either item
        auto l_p = left->get_flux_weight();
        auto r_p = right->get_flux_weight();
        auto total = r_p + l_p;
        //If we hit right it was closer
        if (hit_right) {
            auto p = r_p / total;
            rec = r_rec;
            rec.m_pdf_v *= p;
        }
        else if (hit_left) {
            auto p = l_p / total;
            rec = l_rec;
            rec.m_pdf_v *= p;
        }

        return hit_left || hit_right;
    }

    aabb bounding_box() const override { return bbox; }

    void compute_properties() override {
        left->compute_properties();
        right->compute_properties();
        set_surface_area(left->get_surface_area() + right->get_surface_area());
        set_flux_rgb(left->get_flux_rgb() + right->get_flux_rgb());
    }

    void set_explicit_light(bool is_light) override {
        left->set_explicit_light(is_light);
        right->set_explicit_light(is_light);
    }

    surface_light_sample sample_light_over_flux(double seed, double running_prob) const override {

        double l_flux = left->get_flux_weight();
        double r_flux = right->get_flux_weight();
        double total_flux = l_flux + r_flux;
        double sample = seed * total_flux;

        if (sample < l_flux) {
            auto new_seed = sample / total_flux;
            double prob = l_flux / total_flux;
            return left->sample_light_over_flux(new_seed, running_prob * prob);
        }
        auto new_seed = (sample - l_flux) / total_flux;
        double prob = l_flux / total_flux;
        return right->sample_light_over_flux(new_seed, running_prob * prob);
    }

private:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox;

    static bool box_compare( const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index ) {
        //compare by minimum of ranges
        auto a_axis_interval = a->bounding_box().axis_interval( axis_index );
        auto b_axis_interval = b->bounding_box().axis_interval( axis_index );
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare( const shared_ptr<hittable> a, const shared_ptr<hittable> b ) {
        return box_compare( a, b, 0 );
    }

    static bool box_y_compare( const shared_ptr<hittable> a, const shared_ptr<hittable> b ) {
        return box_compare( a, b, 1 );
    }

    static bool box_z_compare( const shared_ptr<hittable> a, const shared_ptr<hittable> b ) {
        return box_compare( a, b, 2 );
    }
};


#endif //BVH_H
