//
// Created by binjabin on 1/21/26.
//

#ifndef BENDERER_SHAPE_H
#define BENDERER_SHAPE_H
#include "../../structures/vec3.h"
#include "../../records/intersection.h"
#include "../../structures/aabb.h"

//Shape class. These have no transformations, so co-ordinates and vectors are relative to the center
//(ie the shapes are considered to have position and rotation 0)
class shape {
public:
    virtual ~shape() = default;

    virtual bool intersect(const ray& r, interval ray_interval, intersection& isect) const = 0;

    virtual bool intersect_check(const ray& r, interval ray_interval) const = 0;

    virtual float surface_area() const = 0;

    virtual aabb bounding_box() const = 0;

    virtual point3 sample_over_surface() const = 0;

    //The value of the pdf from a point (solid angle)
    virtual double pdf_w_value(const point3& p, const vec3& direction) const = 0;

    //The value of the pdf over area
    virtual double pdf_A_value(const point3& p) const = 0;

    virtual vec3 get_normal(const point3& p) const = 0;

    //Radius of bounding sphere
    virtual double furthest_point() const = 0;
};
#endif //BENDERER_SHAPE_H