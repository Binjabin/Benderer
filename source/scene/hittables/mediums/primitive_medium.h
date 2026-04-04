//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_CONSTANT_MEDIUM_H
#define BENDERER_CONSTANT_MEDIUM_H
#include "medium.h"
#include "../../../records/medium_intersection.h"
#include "../../material/medium_material.h"
#include "../../shapes/solids/solid.h"


class primitive_medium : public medium {
public:
    primitive_medium(std::shared_ptr<solid> boundary, std::shared_ptr<medium_material> mat, int excluder_level = 0)
        : m_boundary(boundary), m_mat(mat), m_excluder_level(excluder_level) {
        set_count(1);
        m_bbox = m_boundary->bounding_box();
        m_furthest_point = boundary->furthest_point();
    }

    //Object functions
    aabb bounding_box() const override {
        return m_bbox;
    }

    bool medium_hit(const ray &r, const interval& r_t, medium_intersections& rec) const override {
        if (bounding_box().hit(r, r_t) == false) return false;

        bool start_inside = m_boundary->contains(r.origin());
        bool intersected = start_inside;

        interval remaining = r_t;

        //We could start inside the medium, in which case we still need to record the time we spend inside
        if (start_inside) {
            if (!process_entry(r, remaining, rec)) {
                //Done already. Never left first volume.
                return true;
            }
        }

        //Now we are outside any intervals. Loop through sections of interval until we don't hit any
        //Need to loop to support non-convex shapes!
        intersection new_entry;
        while (m_boundary->intersect(r, remaining, new_entry)) {
            intersected = true;

            remaining.min = new_entry.get_t();

            if (!process_entry(r, remaining, rec)) {
                //Done. Never this volume.
                return true;
            }
        }

        return intersected;
    }


    void compute_properties() override {
        //Nothing to do
    }

    point3 origin() const override {
        return point3(0,0,0);
    }

    double global_furthest_point() const override {
        return m_furthest_point;
    }

    double local_furthest_point() const override {
        return m_furthest_point;
    }

    std::vector<shared_ptr<medium>> flatten() const override {
        std::vector<shared_ptr<medium>> flattened;
        flattened.push_back(make_shared<primitive_medium>(*this));
        return flattened;
    }
    
private:
    shared_ptr<solid> m_boundary;
    shared_ptr<medium_material> m_mat;

    //Excludes any volumes in the region of lower level
    int m_excluder_level;

    aabb m_bbox;
    double m_furthest_point;


    bool process_entry(const ray& r, interval& remaining, medium_intersections& rec) const {
        interval exit_interval = remaining;
        exit_interval.min += epsilon;

        intersection exit;
        if (!m_boundary->intersect(r, exit_interval, exit)) {
            //Didn't exit and reached end of segment
            rec.add(m_mat, remaining, m_excluder_level);
            return false;
        }

        interval first_part = interval(remaining.min, exit.get_t());
        rec.add(m_mat, first_part, m_excluder_level);

        remaining.min = exit.get_t();
        //Have exitted. More to process
        return true;
    }
};


#endif //BENDERER_CONSTANT_MEDIUM_H