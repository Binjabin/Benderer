//
// Created by binjabin on 1/22/26.
//

#ifndef BENDERER_PSIX_WRITER_H
#define BENDERER_PSIX_WRITER_H

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "image_writer.h"
#include "../../../external/stb_image_write.h"

class psix_writer : public image_writer {
public:
    psix_writer(int w, int h)
        : m_width(w), m_height(h) {
    }

    //Convert accumulated light to pixel values
    void write(const std::string& filename, const std::vector<double>& double_buf) const override {

        if (double_buf.size() != size_t(m_width) * size_t(m_height) * 3)
            throw std::invalid_argument("buffer size mismatch");

        auto name = filename + ".ppm";
        std::ofstream out(name, std::ios::binary);
        if (!out)
            throw std::runtime_error("failed to open file");

        // P6 header
        out << "P6\n" << m_width << " " << m_height << "\n255\n";

        const int pxCount = m_width * m_height;
        for (int i = 0; i < pxCount; ++i) {
            double r = std::clamp(double_buf[i*3 + 0], 0.0, 1.0);
            double g = std::clamp(double_buf[i*3 + 1], 0.0, 1.0);
            double b = std::clamp(double_buf[i*3 + 2], 0.0, 1.0);

            uint8_t rgb[3] = {
                to_byte(r),
                to_byte(g),
                to_byte(b)
            };

            out.write(reinterpret_cast<char*>(rgb), 3);
        }
    }

    std::string extention() override {
        return ".ppm";
    }

private:
    const int m_width, m_height;
    const int m_channels = 3;

    static uint8_t to_byte(double x) {
        return static_cast<uint8_t>(
            std::clamp(int(x * 255.0 + 0.5), 0, 255)
        );
    }

};

#endif //BENDERER_PSIX_WRITER_H