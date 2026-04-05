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

    point3 origin() const override {
        return point3(0,0,0);
    }

    double global_furthest_point() const override {
        return m_furthest_point;
    }

    double local_furthest_point() const override {
        return m_furthest_point;
    }

    void compute_properties() override {
        if (m_mat) {
            double volume = m_boundary->volume();
            set_volume(volume);
            set_flux_rgb(4 * pi * volume * m_mat->average_radiance());
        }
    }

    void set_explicit_light(bool is_light) override {
        m_is_explicit_light = is_light;
    }

    volume_light_sample sample_light_over_flux(double seed, double running_prob) const override {

        vec3 p = m_boundary->sample_over_volume();
        volume_light_sample result;
        result.m_radiance = m_mat->average_radiance();
        result.m_light_p = p;
        result.m_pdf_V = m_boundary->pdf_V_value(p) * running_prob;

        return result;
    }

    double pdf_value(const point3 &origin, const vec3 &direction) const override {
        double V = m_boundary->volume();
        if (V <= 0.0) return 0.0;

        vec3 d = unit_vector(direction);
        ray r(origin, d);

        double integral = 0.0;
        interval remaining = interval(0, infinity);

        if (m_boundary->contains(r.origin())) {
            intersection exit;
            if (!m_boundary->intersect(r, interval(epsilon, infinity), exit)) {
                return 0.0;
            }
            double t_out = exit.get_t();
            //Analytical solve for homogeneous emission, which we use to approximate importance sampling
            integral += t_out * t_out * t_out / 3.0;
            remaining.min = t_out + epsilon;
        }

        intersection entry;
        while (m_boundary->intersect(r, remaining, entry)) {
            double t_in = entry.get_t();

            intersection exit;
            if (!m_boundary->intersect(r, interval(t_in + epsilon, infinity), exit)) {
                break;
            }

            double t_out = exit.get_t();

            integral += (t_out * t_out * t_out - t_in * t_in * t_in) / 3.0;
            remaining.min = t_out + epsilon;
        }

        return integral / V;
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

    bool m_is_explicit_light = false;

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