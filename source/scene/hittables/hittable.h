//
// Created by binjabin on 6/24/25.
//

#ifndef HITTABLE_H
#define HITTABLE_H

#include "../../structures/aabb.h"
#include "../../structures/light_sample.h"

//tell compiler we handle what this is later
class material;

class hit_record {
public:
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t;
    double u;
    double v;
    bool front_face;
    double pdf_v;

    void set_face_normal( const ray& r, const vec3& outward_normal ) {
        //require outward_normal length is 1
        front_face = dot( r.direction(), outward_normal ) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual ~hittable() = default;
    virtual bool hit( const ray& r, interval ray_t, hit_record& rec ) const = 0;
    virtual aabb bounding_box() const = 0;

    virtual double pdf_value(const point3& origin, const vec3& direction) const {
        return 0.0;
    }

    virtual vec3 random(const point3& origin) const {
        return vec3(1, 0, 0);
    }

    virtual int get_count() const {
        return m_count;
    }

    virtual double get_surface_area() const {
        return m_surface_area;
    }

    virtual vec3 get_flux_rgb() const {
        return m_flux_rgb;
    }

    virtual double get_flux_weight() const {
        return flux_weight(m_flux_rgb);
    }

    virtual void compute_properties() = 0;

    virtual shared_ptr<surface_light_sample> sample_light_over_flux(double seed, double running_prob) const = 0;

protected:
    void set_flux_rgb(const vec3& flux_rgb) {
        m_flux_rgb = flux_rgb;
    }

    void set_count(const int count) {
        m_count = count;
    }

    void set_surface_area(const double surface_area) {
        m_surface_area = surface_area;
    }


private:
    int m_count = 0;
    double m_surface_area = 0;
    vec3 m_flux_rgb = vec3(0, 0, 0);
};

#endif //HITTABLE_H
