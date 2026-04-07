//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_SURFACE_H
#define BENDERER_SURFACE_H
#include "../hittable.h"

class surface : public hittable, public std::enable_shared_from_this<surface>{
public:
    virtual bool surface_hit( const ray& r, const interval& ray_t, surface_hit_rec& rec ) const = 0;

    virtual bool surface_hit_check( const ray& r, interval ray_t ) const = 0;

    virtual void set_explicit_light(bool is_light) = 0;

    virtual surface_light_sample sample_light_over_flux(double seed, double running_prob) const = 0;

    virtual double get_surface_area() const {
        return m_surface_area;
    }

    virtual double pdf_value(const point3& origin, const vec3& direction) const {
        return 0.0;
    }

    virtual std::vector<shared_ptr<surface>> flatten() = 0;

protected:

    void set_surface_area(const double surface_area) {
        m_surface_area = surface_area;
    }

private:
    double m_surface_area = 0;
};

#endif //BENDERER_SURFACE_H