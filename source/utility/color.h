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

const vec3 luminance_map = vec3(0.2126, 0.7152, 0.0722);
inline double luminance(const color& rgb) {
    return dot(rgb, luminance_map);
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

namespace colors {
    //Pure colors
    inline static const color white = color(1, 1, 1);
    inline static const color black = color(0,0,0);

    inline static const color red = color(1,0,0);
    inline static const color green = color(0,1,0);
    inline static const color blue = color(0,0,1);

    inline static const color yellow = color(1,1,0);
    inline static const color magenta = color(1,0,1);
    inline static const color cyan = color(0,1,1);

    inline static const color light_gray = color(0.75,0.75,0.75);
    inline static const color gray = color(0.5,0.5,0.5);
    inline static const color dark_gray = color(0.25,0.25,0.25);

    //Nice colors
    inline static const color n_red = color( .65, .05, .05 );
    inline static const color n_green = color( .12, .45, .15 );
    inline static const color n_blue = color(.12, .12, .45);
    inline static const color n_black = color( .1, .1, .1 );
    inline static const color n_white = color( .73, .73, .73 );
    inline static const color n_orange = color( .6, .33, 0 );
    inline static const color n_teal = color( .15, .5, .5 );

    inline static const color metal_grey = color( 0.8, 0.85, 0.88 );
    inline static const color sky = color(0.5,0.7,1.0);

    inline static const color bright_light = color(15, 15, 15);
    inline static const color dim_light = color(4, 4, 4);


};

#endif //COLOR_H
