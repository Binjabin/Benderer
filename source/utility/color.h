//
// Created by binjabin on 6/20/25.
//

#ifndef COLOR_H
#define COLOR_H

#include "../structures/interval.h"
#include "../structures/vec3.h"

using color = vec3;

inline double linear_to_gamma( double linear_component ) {
    if ( linear_component > 0 ) {
        return std::sqrt( linear_component );
    }

    return 0;
}

void write_color( std::ostream& out, const color& pixel_color ) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    //Replace NaN with zero
    if (r != r) r = 0.0;
    if (g != g) g = 0.0;
    if (b != b) b = 0.0;

    r = linear_to_gamma( r );
    g = linear_to_gamma( g );
    b = linear_to_gamma( b );

    static const interval intensity( 0.000, 0.999 );
    int ir = int( 256 * intensity.clamp( r ) );
    int ig = int( 256 * intensity.clamp( g ) );
    int ib = int( 256 * intensity.clamp( b ) );

    out << ir << ' ' << ig << ' ' << ib << '\n';
}


#endif //COLOR_H
