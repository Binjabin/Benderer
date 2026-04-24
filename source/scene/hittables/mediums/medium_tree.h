//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_MEDIUM_TREE_H
#define BENDERER_MEDIUM_TREE_H

#include <algorithm>

#include "medium.h"

//acceleration structure for handling lots of objects in sub-linear time
class medium_tree_node : public medium {
public:

    medium_tree_node( shared_ptr<medium> root ) {
        std::vector<shared_ptr<medium>> flattened = root->flatten();
        build_tree(flattened, 0, flattened.size());
    }

    //construct leaf node. list of hittable objects
    medium_tree_node( std::vector<shared_ptr<medium>>& media, size_t start, size_t end ) {
        build_tree(media, start, end);
    }

    void build_tree(std::vector<shared_ptr<medium>>& media, size_t start, size_t end) {
        //create a bounding box of all source items
        aabb bbox = aabb::empty;
        for ( size_t i = start; i < end; i++ ) {
            bbox = aabb(bbox, media[i]->bounding_box());
        }
        set_bbox(bbox);

        //select random axis to sort along, and get function for sorting
        int axis = bbox.longest_axis();
        auto comparator = ( axis == 0 ) ? box_x_compare : ( ( axis == 1 ) ? box_y_compare : box_z_compare );
        size_t object_span = end - start;

        //if there are 1 or 2 objects, just split them the obvious way, and make the subtrees just the objects
        if ( object_span == 1 ) {
            m_left = m_right = media[start];
        }
        else if ( object_span == 2 ) {
            m_left = media[start];
            m_right = media[start + 1];
        }
        else {
            //sort along some axis (from comparator)
            std::sort( media.begin() + start, media.begin() + end, comparator );
            //then split in through the middle into m_left and m_right subtrees
            auto mid = start + object_span / 2;
            m_left = make_shared<medium_tree_node>( media, start, mid );
            m_right = make_shared<medium_tree_node>( media, mid, end );
        }

        if (m_left == m_right) {
            add_hittable_properties(m_left);
        }
        else {
            add_hittable_properties(m_left);
            add_hittable_properties(m_right);
        }
    }

    //check if we hit any objects in the subtree
    bool medium_hit( const ray& r, const interval& ray_t, medium_intersections& rec ) const override {
        if ( !bounding_box().hit( r, ray_t ) )
            return false;

        medium_intersections l_rec = medium_intersections();
        bool hit_left = m_left->medium_hit( r, ray_t, l_rec );

        medium_intersections r_rec = medium_intersections();
        bool hit_right = m_right->medium_hit( r, ray_t, r_rec );

        if (!hit_left && !hit_right) return false;

        rec.fuse(l_rec);
        rec.fuse(r_rec);
        return true;
    }

    void compute_properties() override {
        m_left->compute_properties();
        m_right->compute_properties();
        set_count(m_left == m_right ? m_left->get_count() : m_left->get_count() + m_right->get_count());
        set_volume(m_left->get_volume() + m_right->get_volume());
        set_flux(m_left->get_flux() + m_right->get_flux());
    }

    void set_explicit_light(bool is_light) override {
        m_left->set_explicit_light(is_light);
        m_right->set_explicit_light(is_light);
    }

    volume_light_sample sample_light_over_flux(double seed, double running_prob) const override {

        double l_flux = m_left->get_flux_weight();
        double r_flux = m_right->get_flux_weight();
        double total_flux = l_flux + r_flux;
        double sample = seed * total_flux;

        if (sample < l_flux) {
            auto new_seed = sample / l_flux;
            double prob = l_flux / total_flux;
            return m_left->sample_light_over_flux(new_seed, running_prob * prob);
        }
        auto new_seed = (sample - l_flux) / r_flux;
        double prob = r_flux / total_flux;
        return m_right->sample_light_over_flux(new_seed, running_prob * prob);
    }

    double pdf_value(const point3 &origin, const vec3 &direction) const override {
        double lflux = m_left->get_flux_weight();
        double rflux = m_right->get_flux_weight();
        double total_flux = lflux + rflux;
        if (total_flux <= 0.0) return 0.0;

        return (lflux / total_flux) * m_left->pdf_value(origin, direction) +
            (rflux / total_flux) * m_right->pdf_value(origin, direction);
    }

    std::vector<shared_ptr<medium>> flatten() override {
        std::vector<shared_ptr<medium>> flattened;
        flattened.insert(flattened.end(), m_left->flatten().begin(), m_left->flatten().end());
        flattened.insert(flattened.end(), m_right->flatten().begin(), m_right->flatten().end());
        return flattened;
    }

private:
    shared_ptr<medium> m_left;
    shared_ptr<medium> m_right;

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