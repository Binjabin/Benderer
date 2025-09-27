//
// Created by binjabin on 9/27/25.
//

#ifndef TEX_CHECKER_H
#define TEX_CHECKER_H

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

#endif //TEX_CHECKER_H
