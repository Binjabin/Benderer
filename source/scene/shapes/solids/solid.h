//
// Created by binjabin on 1/21/26.
//

#ifndef BENDERER_SOLID_H
#define BENDERER_SOLID_H
#include "../shape.h"

//Shapes with volume
class solid : public shape {
public:
    virtual bool contains(const point3& p) const = 0;

    virtual double volume() const = 0;

    virtual point3 sample_over_volume() const = 0;

    virtual double pdf_V_value(const point3& p) const = 0;
};

#endif //BENDERER_SOLID_H