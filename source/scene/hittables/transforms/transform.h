//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_TRANSFORM_H
#define BENDERER_TRANSFORM_H
#include "../hittable.h"

class transform : public hittable {
protected:
    transform(shared_ptr<hittable> object) : m_object(object) {
        if (m_object){
            set_count(m_object->get_count());
            set_surface_area(m_object->get_surface_area());
            set_flux_rgb(m_object->get_flux_rgb());
        }
    }

    shared_ptr<hittable> m_object;

};

#endif //BENDERER_TRANSFORM_H