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
    std::vector<shared_ptr<medium>> mediums;

    medium_list() {
    }

    medium_list( shared_ptr<medium> mediums ) {
        add( mediums );
        set_count(mediums->get_count());
    }

    void clear() { mediums.clear(); }

    void add( shared_ptr<medium> medium ) {
        mediums.push_back( medium );
        bbox = aabb( bbox, medium->bounding_box() );

        if (get_count() == 0) {
            m_origin = medium->origin();
            m_local_furthest_point = medium->local_furthest_point();
        }
        else {
            vec3 offset = medium->origin() - m_origin;
            double dist = offset.length();
            double new_rad = (1.0 / 2.0) * (dist + m_local_furthest_point + medium->local_furthest_point());
            m_origin = m_origin + (1.0 / dist) * (new_rad - m_local_furthest_point) * offset;
            m_local_furthest_point = new_rad;
        }

        m_global_furthest_point = std::max(m_global_furthest_point, medium->global_furthest_point());

        set_count(get_count() + medium->get_count());
    }

    bool medium_hit( const ray& r, const interval& ray_t, medium_intersections& rec ) const override {
        if (mediums.empty()) return false;

        bool hit_anything = false;

        for (int i = 0; i < mediums.size(); i++) {
            auto medium = mediums[i];
            medium_intersections sub_recs;
            if ( medium->medium_hit( r, ray_t, sub_recs ) ) {
                hit_anything = true;
                rec.fuse(sub_recs);
            }
        }

        return hit_anything;
    }

    void compute_properties() override {
        //Nothing to do!
    }

    aabb bounding_box() const override {
        return bbox;
    }

    double global_furthest_point() const override {
        return m_global_furthest_point;
    }

    double local_furthest_point() const override {
        return m_local_furthest_point;
    }

    vec3 origin() const override {
        return m_origin;
    }

private:
    aabb bbox;
    //TODO: Could improve by averaging?
    point3 m_origin = point3(0,0,0);
    double m_local_furthest_point = 0;
    double m_global_furthest_point = 0;

};

#endif //BENDERER_MEDIUM_LIST_H