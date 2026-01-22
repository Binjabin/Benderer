//
// Created by binjabin on 1/22/26.
//

#ifndef BENDERER_POST_PROCESS_H
#define BENDERER_POST_PROCESS_H
#include <algorithm>
#include <cmath>
#include <stdexcept>

//For processing the double buffer before outputting:
class post_process {
public:
    post_process(int w, int h)
        : m_width(w), m_height(h) {
    }

    void apply_post(std::vector<double>& double_buf ) {

        //Not sure what this is??

        const int pxCount = m_height * m_width;

        for (int i = 0; i < pxCount; ++i) {
            double r = sanitize(double_buf[i*3 + 0]) * exposure_mul;
            double g = sanitize(double_buf[i*3 + 1]) * exposure_mul;
            double b = sanitize(double_buf[i*3 + 2]) * exposure_mul;

            // clamp then gamma
            r = std::clamp(r, 0.0, 1.0);
            g = std::clamp(g, 0.0, 1.0);
            b = std::clamp(b, 0.0, 1.0);

            //Tonemap
            r = tonemap(r);
            g = tonemap(g);
            b = tonemap(b);

            if (m_gamma > 0.0) {
                r = gamma(r);
                g = gamma(g);
                b = gamma(b);
            }

            const int base = i * m_channels;
            double_buf[base + 0] = r;
            double_buf[base + 1] = g;
            double_buf[base + 2] = b;
        }
    }



private:
    int m_width, m_height;
    const int m_channels = 3;
    const double m_gamma = 2.2;
    const double exposure_mul = 2.0;

    //removes any infinite or negative values
    static double sanitize(const double v) {
        return (std::isfinite(v) && v > 0.0) ? v : 0.0;
    }

    double gamma(double v) const {
        return std::pow(v, 1.0 / m_gamma);
    }

    double tonemap(double x) {
        return x / (1.0 + x);
    }
};

#endif //BENDERER_POST_PROCESS_H