//
// Created by binjabin on 9/27/25.
//

#ifndef TEX_IMAGE_H
#define TEX_IMAGE_H

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

#endif //TEX_IMAGE_H
