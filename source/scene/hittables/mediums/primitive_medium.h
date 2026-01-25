//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_CONSTANT_MEDIUM_H
#define BENDERER_CONSTANT_MEDIUM_H
#include "medium.h"
#include "../../../records/medium_hit_rec.h"
#include "../../material/medium_material.h"
#include "../../shapes/solids/solid.h"


class primitive_medium : public medium {
public:
    primitive_medium(std::shared_ptr<solid> boundary, std::shared_ptr<medium_material> mat)
        : m_boundary(boundary), m_mat(mat){
        set_count(1);
        m_bbox = m_boundary->bounding_box();
    }

    //Object functions
    aabb bounding_box() const override {
        return m_bbox;
    }

    bool medium_hit(const ray &r, const interval& r_t, medium_hit_rec &rec) const override {
        bool start_inside = m_boundary->contains(r.origin());


        double start_t = 0.0;
        double after_entry_t = 0.0;

        if (!start_inside) {

            intersection entry;
            if (!m_boundary->intersect(r, r_t, entry)) {
                return false;
            }

            start_t = entry.get_t();
            after_entry_t = start_t + epsilon;
        }

        //find exit
        intersection exit;
        interval r2_t = interval(after_entry_t, r_t.max);
        ray r2 = ray(r.origin(), r.direction());
        if (!m_boundary->intersect(r2, r2_t, exit)) {
            //Shouldn't happen!
            return false;
        }

        interval medium_interval = interval(start_t, exit.get_t());

        m_mat->sample(r, medium_interval, rec);

        return true;
    }

    void compute_properties() override {
        //Nothing to do
    }

private:
    shared_ptr<solid> m_boundary;
    shared_ptr<medium_material> m_mat;
    aabb m_bbox;
};


#endif //BENDERER_CONSTANT_MEDIUM_H