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
const double infinity = std::numeric_limits<double>::infinity();
const double epsilon = 1e-8;
const double pi = 3.1415926535897932385;

//INLINES

inline double degrees_to_radians( double degrees ) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution( 0.0, 1.0 );
    static std::mt19937 generator;
    return distribution( generator );
}

inline double random_double(double min, double max) {
    return min + ( max - min ) * random_double();
}

inline int random_int(int min, int max) {
    return int(random_double(min, max+1));
}

//COMMON HEADERS

#include "utility/color.h"
#include "structures/ray.h"
#include "structures/vec3.h"
#include "structures/interval.h"

#endif //BENDERER_H
