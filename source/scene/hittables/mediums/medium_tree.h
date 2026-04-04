//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_MEDIUM_TREE_H
#define BENDERER_MEDIUM_TREE_H

#include <algorithm>

#include "medium.h"
#include "surface.h"
#include "surface_list.h"

//acceleration structure for handling lots of objects in sub-linear time
class medium_tree_node : public medium {
public:

    medium_tree_node( shared_ptr<medium> root ) {
        std::vector<shared_ptr<medium>> flattened = root->flatten();
        *this = medium_tree_node( flattened, 0, flattened.size() );
    }

    //construct leaf node. list of hittable objects
    medium_tree_node( std::vector<shared_ptr<medium>>& surfaces, size_t start, size_t end ) {
        //create a bounding box of all source items
        bbox = aabb::empty;
        for ( size_t object_index = start; object_index < end; object_index++ ) {
            bbox = aabb(bbox, surfaces[object_index]->bounding_box());
        }

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
            std::sort( std::begin( surfaces ) + start, std::begin( surfaces ) + end, comparator );
            //then split in through the middle into left and right subtrees
            auto mid = start + object_span / 2;
            left = make_shared<medium_tree_node>( surfaces, start, mid );
            right = make_shared<medium_tree_node>( surfaces, mid, end );
        }

        //Bounding sphere
        vec3 offset = left->origin() - right->origin();
        double dist = offset.length();
        double new_rad = (1.0 / 2.0) * (dist  + left->local_furthest_point() + right->local_furthest_point());
        m_origin = left->origin() + (1.0 / dist) * (new_rad - left->local_furthest_point()) * offset;
        m_local_furthest_point = new_rad;
        m_global_furthest_point = std::max(right->global_furthest_point(), left->global_furthest_point());

        int count_sum = 0;
        double area_sum = 0;
        vec3 flux_sum = vec3(0,0,0);
        for ( size_t i = start; i < end; i++ ) {
            count_sum += surfaces[i]->get_count();
        }
        set_count(count_sum);
    }

    //check if we hit any objects in the subtree
    bool medium_hit( const ray& r, interval ray_t, medium_intersections& rec ) const override {
        if ( !bbox.hit( r, ray_t ) )
            return false;

        medium_intersections l_rec = medium_intersections();
        bool hit_left = left->medium_hit( r, ray_t, l_rec );

        medium_intersections r_rec = medium_intersections();
        interval right_ray_t = interval( ray_t.min, hit_left ? l_rec.t() : ray_t.max );
        bool hit_right = right->medium_hit( r, right_ray_t, r_rec );

        if (!hit_left && !hit_right) return false;

        rec.fuse(l_rec);
        rec.fuse(r_rec);
        return true;
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

    std::vector<shared_ptr<medium>> flatten() const override {
        std::vector<shared_ptr<medium>> flattened;
        flattened.insert(flattened.end(), left->flatten().begin(), left->flatten().end());
        flattened.insert(flattened.end(), right->flatten().begin(), right->flatten().end());
        return flattened;
    }

private:
    shared_ptr<medium> left;
    shared_ptr<medium> right;
    aabb bbox;

    point3 m_origin;
    double m_local_furthest_point;
    double m_global_furthest_point;

    static bool box_compare( const shared_ptr<medium> a, const shared_ptr<medium> b, int axis_index ) {
        //compare by minimum of ranges
        auto a_axis_interval = a->bounding_box().axis_interval( axis_index );
        auto b_axis_interval = b->bounding_box().axis_interval( axis_index );
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare( const shared_ptr<medium> a, const shared_ptr<medium> b ) {
        return box_compare( a, b, 0 );
    }

    static bool box_y_compare( const shared_ptr<medium> a, const shared_ptr<medium> b ) {
        return box_compare( a, b, 1 );
    }

    static bool box_z_compare( const shared_ptr<medium> a, const shared_ptr<medium> b ) {
        return box_compare( a, b, 2 );
    }


};

#endif //BENDERER_MEDIUM_TREE_H