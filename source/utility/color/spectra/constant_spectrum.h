//
// Created by binjabin on 2/12/26.
//

#ifndef BENDERER_CONSTANT_SPECTRUM_H
#define BENDERER_CONSTANT_SPECTRUM_H
#include "../spectrum.h"

class constant_spectrum : public spectrum{
public:
    constant_spectrum(double c)
        : m_c(c) {
    }

    double operator()(double lambda) const override {
        return m_c;
    }

    double max_value() const override {
        return m_c;
    }

private:
    double m_c = 0.0;
};

#endif //BENDERER_CONSTANT_SPECTRUM_H