//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_TRANSFORM_H
#define BENDERER_TRANSFORM_H
#include "../hittable.h"

class transform : public hittable {
public:
    void compute_properties() override {
        if (m_object){
            m_object->compute_properties();
            set_count(m_object->get_count());
            set_surface_area(m_object->get_surface_area());
            set_flux_rgb(m_object->get_flux_rgb());
        }
    }
protected:
    transform(shared_ptr<hittable> object) : m_object(object) {
    }

    shared_ptr<hittable> m_object;

};

#endif //BENDERER_TRANSFORM_H