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

    virtual aabb bounding_box() const = 0;

    virtual int get_count() const {
        return m_count;
    }

    void set_count(const int count) {
        m_count = count;
    }

private:
    int m_count = 0;

};

#endif //HITTABLE_H
