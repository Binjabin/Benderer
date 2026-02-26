//
// Created by binjabin on 2/12/26.
//

#ifndef BENDERER_SPECTRUM_H
#define BENDERER_SPECTRUM_H

//Visible light range
constexpr double lambda_min = 360.0, lambda_max = 830.0;

class spectrum {
public:
    virtual spectrum() = default;
    virtual ~spectrum() = default;

    virtual double operator()(double lambda) const = 0;
    //The highest value of spectral distribution over the wavelength
    virtual double max_value() const = 0;
};

#endif //BENDERER_SPECTRUM_H