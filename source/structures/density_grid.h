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

    void precompute() {
        auto safe = [](double s){return s > 0.0 ? s : 1.0;};
        kx = (nx - 1) / safe(bounds.x.size());
        ky = (ny - 1) / safe(bounds.y.size());
        kz = (nz - 1) / safe(bounds.z.size());
        ox = -bounds.x.min * kx;
        oy = -bounds.y.min * ky;
        oz = -bounds.z.min * kz;
    }

    float at(int i, int j, int k) const {
        return data[i + j * nx + k * nx * ny];
    }

    double sample_density(const point3& p) const {
        if (nx < 2 || ny < 2 || nz < 2) return (nx > 0 && ny > 0 && nz > 0) ? at(0, 0, 0) : 0.0;

        const double x_raw = p.x() * kx + ox;
        const double y_raw = p.y() * ky + oy;
        const double z_raw = p.z() * kz + oz;

        const double x = std::max(0.0, std::min((double)nx - 1.0001, x_raw));
        const double y = std::max(0.0, std::min((double)ny - 1.0001, y_raw));
        const double z = std::max(0.0, std::min((double)nz - 1.0001, z_raw));

        const int i = (int)x;
        const int j = (int)y;
        const int k = (int)z;

        const double dx = x - i;
        const double dy = y - j;
        const double dz = z - k;

        const int nxny = nx * ny;
        const int base_idx = i + j * nx + k * nxny;
        const float* d_ptr = data.data() + base_idx;

        const double c000 = d_ptr[0];
        const double c100 = d_ptr[1];
        const double c010 = d_ptr[nx];
        const double c110 = d_ptr[nx + 1];
        const double c001 = d_ptr[nxny];
        const double c101 = d_ptr[nxny + 1];
        const double c011 = d_ptr[nxny + nx];
        const double c111 = d_ptr[nxny + nx + 1];

        const double c00 = c000 + dx * (c100 - c000);
        const double c10 = c010 + dx * (c110 - c010);
        const double c01 = c001 + dx * (c101 - c001);
        const double c11 = c011 + dx * (c111 - c011);

        const double c0 = c00 + dy * (c10 - c00);
        const double c1 = c01 + dy * (c11 - c01);

        return c0 + dz * (c1 - c0);
    }

private:
    double kx, ky, kz;
    double ox, oy, oz;

};

#endif //BENDERER_DENSITY_GRID_H
