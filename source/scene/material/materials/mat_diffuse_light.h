//
// Created by binjabin on 9/27/25.
//

#ifndef MAT_DIFFUSE_LIGHT_H
#define MAT_DIFFUSE_LIGHT_H

class diffuse_light : public material {
public:
    diffuse_light( shared_ptr<texture> tex ) : tex( tex ) {
    }

    diffuse_light( const color& emit ) : tex( make_shared<solid_color>( emit ) ) {
    }

    color emitted( const ray& r_in, const hit_record& rec, double u, double v, const point3& p ) const override {
        if ( !rec.front_face ) {
            return color( 0, 0, 0 );
        }
        return tex->value( u, v, p );
    }

private:
    shared_ptr<texture> tex;
};

#endif //MAT_DIFFUSE_LIGHT_H
