//
// Created by binjabin on 12/8/25.
//

#ifndef BENDERER_ROTATE_Y_H
#define BENDERER_ROTATE_Y_H

class rotate_y : public transform {
public:
    rotate_y( shared_ptr<hittable> object, double angle ) : transform( object ) {
        auto radians = degrees_to_radians( angle );
        sin_theta = std::sin( radians );
        cos_theta = std::cos( radians );
        bbox = object->bounding_box();

        point3 min = point3( infinity, infinity, infinity );
        point3 max = point3( -infinity, -infinity, -infinity );

        //find min and max in each dimension, checking each corner
        for ( int i = 0; i < 2; i++ ) {
            for ( int j = 0; j < 2; j++ ) {
                for ( int k = 0; k < 2; k++ ) {
                    auto x = i * bbox.x.max + ( 1 - i ) * bbox.x.min;
                    auto y = j * bbox.y.max + ( 1 - j ) * bbox.y.min;
                    auto z = k * bbox.z.max + ( 1 - k ) * bbox.z.min;

                    auto newx = cos_theta * x - sin_theta * z;
                    auto newz = sin_theta * x + cos_theta * z;

                    vec3 tester( newx, y, newz );

                    for ( int c = 0; c < 3; c++ ) {
                        min[c] = std::fmin( min[c], tester[c] );
                        max[c] = std::fmax( max[c], tester[c] );
                    }
                }
            }
        }

        bbox = aabb( min, max );
    }

    aabb bounding_box() const override { return bbox; }

    ray transform_ray(const ray& r) const override {
        auto ox = cos_theta * r.origin().x() + sin_theta * r.origin().z();
        auto oz = -sin_theta * r.origin().x() + cos_theta * r.origin().z();
        auto origin = point3( ox, r.origin().y(), oz );

        auto dx = cos_theta * r.direction().x() + sin_theta * r.direction().z();
        auto dz = -sin_theta * r.direction().x() + cos_theta * r.direction().z();
        auto direction = vec3( dx, r.direction().y(), dz );

        return ray( origin, direction, r.time() );
    }

    point3 reverse_transform_point(const point3& p) const override {
        auto px = cos_theta * p.x() - sin_theta * p.z();
        auto pz = sin_theta * p.x() + cos_theta * p.z();
        return point3( px, p.y(), pz );
    }

    vec3 reverse_transform_normal(const vec3& normal) const override {
        auto nx = cos_theta * normal.x() - sin_theta * normal.z();
        auto nz = sin_theta * normal.x() + cos_theta * normal.z();
        return vec3( nx, normal.y(), nz );
    }

private:
    double sin_theta;
    double cos_theta;
    aabb bbox;
};

#endif //BENDERER_ROTATE_Y_H