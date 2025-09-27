//
// Created by binjabin on 9/27/25.
//

#ifndef TEX_NOISE_H
#define TEX_NOISE_H

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


#endif //TEX_NOISE_H
