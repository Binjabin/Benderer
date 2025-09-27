//
// Created by binjabin on 7/8/25.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include "benderer_stb_image.h"
#include "perlin.h"
#include "vec3.h"


class texture {
public:
    virtual ~texture() = default;
    virtual color value( double u, double v, const point3& p ) const = 0;
};


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


class checker_texture : public texture {
public:
    checker_texture( double scale, shared_ptr<texture> even, shared_ptr<texture> odd )
        : inv_scale( 1.0 / scale ), even( even ), odd( odd ) {
    }

    checker_texture( double scale, color even, color odd )
        : inv_scale( 1.0 / scale ), even( make_shared<solid_color>( even ) ), odd( make_shared<solid_color>( odd ) ) {
    }

    color value( double u, double v, const point3& p ) const override {
        auto xInteger = int( std::floor( inv_scale * p.x() ) );
        auto yInteger = int( std::floor( inv_scale * p.y() ) );
        auto zInteger = int( std::floor( inv_scale * p.z() ) );

        bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

        return isEven ? even->value( u, v, p ) : odd->value( u, v, p );
    }

private:
    double inv_scale;
    shared_ptr<texture> even;
    shared_ptr<texture> odd;
};

class image_texture : public texture {
public:
    image_texture(const char* filename) : image(filename) {}

    color value(double u, double v, const point3& p) const override {
        if (image.height() <= 0) return color(0, 1, 1);

        u = interval(0, 1).clamp( u );
        v = 1.0 - interval(0, 1).clamp( v );

        auto i = int(u * image.width());
        auto j = int(v * image.height());
        auto pixel = image.pixel_data( i, j );

        auto color_scale = 1.0 / 255.0;
        return color(color_scale*pixel[0], color_scale*pixel[1], color_scale*pixel[2]);
    }

private:
    benderer_image image;
};

class noise_texture : public texture {
public:
    noise_texture(double scale) : scale(scale) {}

    color value(double u, double v, const point3& p) const override {
        return color(.5, .5, .5) * (1 + std::sin(scale * p.z() + 10 * noise.turb(p, 7)));
    }
private:
    perlin noise;
    double scale;
};

#endif //TEXTURE_H
