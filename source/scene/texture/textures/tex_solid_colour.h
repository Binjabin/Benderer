//
// Created by binjabin on 9/27/25.
//

#ifndef TEX_SOLID_COLOUR_H
#define TEX_SOLID_COLOUR_H

class solid_color : public texture {
public:
    solid_color( const color& albedo ) : albedo( albedo ) {
    }

    solid_color( double red, double green, double blue )
        : albedo( color( red, green, blue ) ) {
    }

    color value( double u, double v, const point3& p ) const override {
        return albedo;
    }

private:
    color albedo;
};

#endif //TEX_SOLID_COLOUR_H
