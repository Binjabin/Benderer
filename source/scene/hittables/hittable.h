//
// Created by binjabin on 6/24/25.
//

#ifndef HITTABLE_H
#define HITTABLE_H

#include "../../structures/aabb.h"
#include "../../records/light_sample.h"
#include "../../records/surface_hit_rec.h"

class hittable {
public:
    virtual ~hittable() = default;
    virtual void compute_properties() = 0;

    aabb bounding_box() const { return m_bbox; }
    int get_count() const { return m_count; }
    point3 origin() const { return m_origin; }
    double global_furthest_point() const { return m_global_furthest_point; }
    double local_furthest_point() const { return m_local_furthest_point; }
    color get_flux() const { return m_flux; }
    double get_flux_weight() const { return luminance(m_flux); };

protected:
    void set_count(int count) { m_count = count; }
    void set_bbox(const aabb& bbox) { m_bbox = bbox; }
    void set_origin(const point3& origin) { m_origin = origin; }
    void set_local_furthest_point(const double local_furthest_point) { m_local_furthest_point = local_furthest_point; }
    void set_global_furthest_point(const double global_furthest_point) { m_global_furthest_point = global_furthest_point; }
    void set_flux(const color& flux_rgb) { m_flux = flux_rgb; }

    //Used for collections
    void add_hittable_properties(const shared_ptr<hittable>& other) {
        m_bbox = aabb(m_bbox, other->bounding_box());

        if (m_count == 0) {
            m_origin = other->origin();
            m_local_furthest_point = other->local_furthest_point();
        }
        else {
            vec3 offset = other->origin() - m_origin;
            double dist = offset.length();

            if (dist + other->local_furthest_point() <= m_local_furthest_point) {
                //Already contained, skip
            }
            else if (dist + m_local_furthest_point <= other->local_furthest_point()) {
                //We are contained, replace
                m_origin = other->origin();
                m_local_furthest_point = other->local_furthest_point();
            }
            else {
                //Expand to contain both
                double new_rad = (1.0 / 2.0) * (dist + m_local_furthest_point + other->local_furthest_point());
                m_origin = m_origin + (1.0 / dist) * (new_rad - m_local_furthest_point) * offset;
                m_local_furthest_point = new_rad;
            }
        }

        m_count += other->get_count();
        m_global_furthest_point = std::max(m_global_furthest_point, other->global_furthest_point());
    }

private:
    int m_count = 0;
    aabb m_bbox = aabb::empty;
    point3 m_origin = point3(0,0,0);
    double m_local_furthest_point = 0;
    double m_global_furthest_point = 0;
    color m_flux = color(0, 0, 0);

};

#endif //HITTABLE_H
