//
// Created by binjabin on 1/23/26.
//

#ifndef BENDERER_SURFACE_H
#define BENDERER_SURFACE_H
#include "../hittable.h"

class surface : public hittable {
public:
    virtual bool surface_hit( const ray& r, interval ray_t, surface_hit_rec& rec ) const = 0;

    virtual void set_explicit_light(bool is_light) = 0;

    virtual surface_light_sample sample_light_over_flux(double seed, double running_prob) const = 0;

    virtual double get_surface_area() const {
        return m_surface_area;
    }

    virtual color get_flux_rgb() const {
        return m_flux_rgb;
    }

    virtual double pdf_value(const point3& origin, const vec3& direction) const {
        return 0.0;
    }

    virtual double get_flux_weight() const {
        return luminance(m_flux_rgb);
    }

protected:
    void set_flux_rgb(const vec3& flux_rgb) {
        m_flux_rgb = flux_rgb;
    }

    void set_surface_area(const double surface_area) {
        m_surface_area = surface_area;
    }

private:
    double m_surface_area = 0;
    color m_flux_rgb = vec3(0, 0, 0);
};

#endif //BENDERER_SURFACE_H