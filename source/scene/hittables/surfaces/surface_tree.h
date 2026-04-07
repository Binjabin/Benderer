//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_SURFACE_TREE_H
#define BENDERER_SURFACE_TREE_H


#include <algorithm>

#include "surface.h"
#include "surface_list.h"

//acceleration structure for handling lots of objects in sub-linear time
class surface_tree_node : public surface {
public:

    surface_tree_node( shared_ptr<surface> root ) {
        std::vector<shared_ptr<surface>> flattened = root->flatten();
        build_tree(flattened, 0, flattened.size());
    }

    //construct leaf node. list of hittable objects
    surface_tree_node( std::vector<shared_ptr<surface>>& surfaces, size_t start, size_t end ) {
        build_tree(surfaces, start, end);
    }

    void build_tree(std::vector<shared_ptr<surface>>& surfaces, size_t start, size_t end) {
        aabb bbox = aabb::empty;
        for ( size_t i = start; i < end; i++ ) {
            bbox = aabb(bbox, surfaces[i]->bounding_box());
        }
        set_bbox(bbox);

        //select random axis to sort along, and get function for sorting
        int axis = bbox.longest_axis();
        auto comparator = ( axis == 0 ) ? box_x_compare : ( ( axis == 1 ) ? box_y_compare : box_z_compare );
        size_t object_span = end - start;

        //if there are 1 or 2 objects, just split them the obvious way, and make the subtrees just the objects
        if ( object_span == 1 ) {
            left = right = surfaces[start];
        }
        else if ( object_span == 2 ) {
            left = surfaces[start];
            right = surfaces[start + 1];
        }
        else {
            //sort along some axis (from comparator)
            std::sort( surfaces.begin() + start, surfaces.begin() + end, comparator );
            //then split in through the middle into left and right subtrees
            auto mid = start + object_span / 2;
            left = make_shared<surface_tree_node>( surfaces, start, mid );
            right = make_shared<surface_tree_node>( surfaces, mid, end );
        }

        if (left == right) {
            add_hittable_properties(left);
        }
        else {
            add_hittable_properties(left);
            add_hittable_properties(right);
        }
    }

    //check if we hit any objects in the subtree
    bool surface_hit( const ray& r, const interval& ray_t, surface_hit_rec& rec ) const override {
        if ( !bounding_box().hit( r, ray_t ) )
            return false;

        surface_hit_rec l_rec = surface_hit_rec();
        bool hit_left = left->surface_hit( r, ray_t, l_rec );

        surface_hit_rec r_rec = surface_hit_rec();
        interval right_ray_t = interval( ray_t.min, hit_left ? l_rec.get_t() : ray_t.max );
        bool hit_right = right->surface_hit( r, right_ray_t, r_rec );

        if (!hit_left && !hit_right) return false;

        //If we hit right it was closer
        if (hit_right) rec = r_rec;
        else if (hit_left) rec = l_rec;

        return true;
    }

    bool surface_hit_check( const ray& r, interval ray_t ) const override {
        if ( !bounding_box().hit( r, ray_t ) )
            return false;

        bool hit_left = left->surface_hit_check( r, ray_t );
        if (hit_left) return true;
        return right->surface_hit_check( r, ray_t );
    }

    void compute_properties() override {
        left->compute_properties();
        right->compute_properties();
        set_count(left == right ? left->get_count() : left->get_count() + right->get_count());
        set_surface_area(left->get_surface_area() + right->get_surface_area());
        set_flux(left->get_flux() + right->get_flux());
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
            auto new_seed = sample / l_flux;
            double prob = l_flux / total_flux;
            return left->sample_light_over_flux(new_seed, running_prob * prob);
        }
        auto new_seed = (sample - l_flux) / r_flux;
        double prob = r_flux / total_flux;
        return right->sample_light_over_flux(new_seed, running_prob * prob);
    }

    std::vector<shared_ptr<surface>> flatten() override {
        std::vector<shared_ptr<surface>> flattened;
        flattened.insert(flattened.end(), left->flatten().begin(), left->flatten().end());
        flattened.insert(flattened.end(), right->flatten().begin(), right->flatten().end());
        return flattened;
    }

    double pdf_value(const point3 &origin, const vec3 &direction) const override {
        double lflux = left->get_flux_weight();
        double rflux = right->get_flux_weight();
        double total_flux = lflux + rflux;
        if (total_flux <= 0.0) return 0.0;

        return (lflux / total_flux) * left->pdf_value(origin, direction) +
            (rflux / total_flux) * right->pdf_value(origin, direction);
    }

private:
    shared_ptr<surface> left;
    shared_ptr<surface> right;

    static bool box_compare( const shared_ptr<surface> a, const shared_ptr<surface> b, int axis_index ) {
        //compare by minimum of ranges
        auto a_axis_interval = a->bounding_box().axis_interval( axis_index );
        auto b_axis_interval = b->bounding_box().axis_interval( axis_index );
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare( const shared_ptr<surface> a, const shared_ptr<surface> b ) {
        return box_compare( a, b, 0 );
    }

    static bool box_y_compare( const shared_ptr<surface> a, const shared_ptr<surface> b ) {
        return box_compare( a, b, 1 );
    }

    static bool box_z_compare( const shared_ptr<surface> a, const shared_ptr<surface> b ) {
        return box_compare( a, b, 2 );
    }
};

#endif //BENDERER_SURFACE_TREE_H