//
// Created by binjabin on 1/22/26.
//

#ifndef BENDERER_PNG_WRITER_H
#define BENDERER_PNG_WRITER_H

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "../../../external/stb_image_write.h"

class png_writer : public image_writer {
public:
    png_writer(int w, int h)
        : m_width(w), m_height(h) {
    }

    //Convert accumulated light to pixel values
    void write(const std::string& filename, const std::vector<double>& double_buf) const override {

        auto data = double_buf.data();
        if (!data) throw std::invalid_argument("float_buf is null");

        std::vector<uint8_t> byte_buffer;
        byte_buffer.resize( size_t(m_width) * size_t(m_height) * 3 );

        const int pxCount = m_height * m_width;

        for (int i = 0; i < pxCount; ++i) {
            double r = double_buf[i*3 + 0];
            double g = double_buf[i*3 + 1];
            double b = double_buf[i*3 + 2];

            //clamp
            r = std::clamp(r, 0.0, 1.0);
            g = std::clamp(g, 0.0, 1.0);
            b = std::clamp(b, 0.0, 1.0);

            const int base = i * m_channels;
            byte_buffer[base + 0] = double_to_byte(r);
            byte_buffer[base + 1] = double_to_byte(g);
            byte_buffer[base + 2] = double_to_byte(b);
        }

        int stride = m_width * m_channels;
        std::string name_with_tag = filename + ".png";
        const char* name = name_with_tag.data();
        stbi_write_png(name, m_width, m_height, 3, byte_buffer.data(), stride);
    }

    std::string file_extention() override {
        return ".png";
    }

private:
    const int m_width, m_height;
    const int m_channels = 3;

    static uint8_t double_to_byte(const double x) {
        int iv = static_cast<int>(std::floor(255.0f * std::clamp(x, 0.0, 1.0) + 0.5f));
        if (iv < 0) iv = 0; if (iv > 255) iv = 255;
        return static_cast<uint8_t>(iv);
    }


};

#endif //BENDERER_PNG_WRITER_H