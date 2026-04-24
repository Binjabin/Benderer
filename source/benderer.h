//
// Created by binjabin on 6/24/25.
//

#ifndef BENDERER_H
#define BENDERER_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

#include <random>

using std::make_shared;
using std::shared_ptr;

//CONSTS
constexpr double infinity = std::numeric_limits<double>::infinity();
constexpr double epsilon = 1e-8;
constexpr double pi = 3.1415926535897932385;
constexpr double inv_pi = 1.0 / pi;

constexpr double uninit = std::numeric_limits<double>::quiet_NaN();

//INLINES

inline double degrees_to_radians( double degrees ) {
    return degrees * pi / 180.0;
}

inline double linear_interp(double a, double b, double t) {
    return a * (1.0 - t) + b * t;
}

inline double random_double() {
    static thread_local std::uniform_real_distribution<double> distribution( 0.0, 1.0 );
    static thread_local std::mt19937 generator{std::random_device{}()};
    return distribution( generator );
}

inline double random_double(double min, double max) {
    return min + ( max - min ) * random_double();
}

inline int random_int(int min, int max) {
    return int(random_double(min, max+1));
}

//COMMON HEADERS

#include "utility/color/color.h"
#include "structures/ray.h"
#include "structures/vec3.h"
#include "structures/interval.h"

#endif //BENDERER_H
