//
// Created by binjabin on 4/5/26.
//

#ifndef BENDERER_DENSITY_GRID_H
#define BENDERER_DENSITY_GRID_H
#include <math.h>
#include <vector>

#include "aabb.h"

class density_grid {
public:
    int nx, ny, nz;
    std::vector<float> data;

    aabb bounds;

    float at(int i, int j, int k) const {
        return data[i + j * nx + k * nx * ny];
    }

    double sample_density(const point3& p) const {
        const vec3 local = bounds.offset(p);

        const double x = local.x() * (nx - 1);
        const double y = local.y() * (ny - 1);
        const double z = local.z() * (nz - 1);

        const int i = floor(x);
        const int j = floor(y);
        const int k = floor(z);

        const double dx = x - i;
        const double dy = y - j;
        const double dz = z - k;

        const double c000 = at(i, j, k);
        const double c100 = at(i + 1, j, k);
        const double c010 = at(i, j + 1, k);
        const double c001 = at(i, j, k + 1);
        const double c110 = at(i + 1, j + 1, k);
        const double c101 = at(i + 1, j, k + 1);
        const double c011 = at(i, j + 1, k + 1);
        const double c111 = at(i + 1, j + 1, k + 1);

        const double c00 = lerp(c000, c100, dx);
        const double c10 = lerp(c010, c110, dx);
        const double c01 = lerp(c001, c101, dx);
        const double c11 = lerp(c011, c111, dx);

        const double c0 = lerp(c00, c10, dy);
        const double c1 = lerp(c01, c11, dy);

        const double u = lerp(c0, c1, dz);

        return u;
    }

};

#endif //BENDERER_DENSITY_GRID_H
