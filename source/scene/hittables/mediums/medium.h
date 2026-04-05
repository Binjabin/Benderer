//
// Created by binjabin on 1/18/26.
//

#ifndef BENDERER_MEDIUM_H
#define BENDERER_MEDIUM_H

#include "../hittable.h"
#include "../../../records/medium_intersection.h"
#include "../../../structures/ray.h"
#include "../../../utility/color/color.h"

class medium : public hittable {
public:
    virtual ~medium() = default;

    virtual bool medium_hit(const ray& r, const interval& ray_t, medium_intersections& rec) const = 0;

    virtual void set_explicit_light(bool is_light) = 0;

    virtual volume_light_sample sample_light_over_flux(double seed, double running_prob) const = 0;

    virtual double get_volume() const {
        return m_volume;
    }

    virtual color get_flux() const {
        return m_flux;
    }

    virtual double pdf_value(const point3& origin, const vec3& direction) const {
        return 0.0;
    }

    virtual double get_flux_lum() const {
        return luminance(m_flux);
    }

    virtual std::vector<shared_ptr<medium>> flatten() const = 0;

protected:
    void set_flux_rgb(const vec3& flux_rgb) {
        m_flux = flux_rgb;
    }

    void set_volume(const double volume) {
        m_volume = volume;
    }

private:
    double m_volume = 0;
    color m_flux = vec3(0, 0, 0);


};

#endif //BENDERER_MEDIUM_H