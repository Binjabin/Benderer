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
        set_count(get_count() + medium->get_count());
    }

    bool medium_hit( const ray& r, const interval& ray_t, medium_hit_rec& rec ) const override {
        if (mediums.empty()) return false;

        medium_hit_rec temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        //Accumulate across all
        auto hit_medium = -1;
        auto earliest_t = infinity;

        for (int i = 0; i < mediums.size(); i++) {
            auto medium = mediums[i];

            if ( medium->medium_hit( r, ray_t, temp_rec ) ) {
                hit_anything = true;
                auto this_t = temp_rec.m_t();
                if (this_t < earliest_t) {
                    earliest_t = this_t;
                    hit_medium = i;
                    rec = temp_rec;
                }
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

private:
    aabb bbox;
};

#endif //BENDERER_MEDIUM_LIST_H