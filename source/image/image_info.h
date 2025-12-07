//
// Created by binjabin on 12/7/25.
//

#ifndef BENDERER_IMAGE_INFO_H
#define BENDERER_IMAGE_INFO_H

class image_info {
public:
    image_info(double aspect_ratio, int pixel_width, int samples_per_pixel, int max_depth)
        : m_aspect_ratio(aspect_ratio),
          m_pixel_width(pixel_width),
          m_samples_per_pixel(samples_per_pixel),
          m_max_depth(max_depth) {
        validate();
    }

    //Getters
    double aspect_ratio() const { return m_aspect_ratio; }
    int pixel_width() const { return m_pixel_width; }
    int pixel_height() const { return m_pixel_height; }
    int samples_per_pixel() const { return m_samples_per_pixel; }
    int max_depth() const { return m_max_depth; }
    double sample_scale() const { return m_sample_scale; }
    double actual_ratio() const { return m_actual_ratio; }


private:
    double m_aspect_ratio; // Ratio of image width over height
    int m_pixel_width; //Width of image in pixels
    int m_samples_per_pixel; // Count of random samples of each pixel
    int m_max_depth; // Maximum number of ray bounces into a scene

    int m_pixel_height;
    double m_sample_scale;
    double m_actual_ratio;

    //Computed
    int compute_pixel_height() const {
        int height = int(m_pixel_width / m_aspect_ratio);
        return ( height < 1 ) ? 1 : height;
    }

    double compute_sample_scale() const {
        return 1.0 / m_samples_per_pixel;
    }

    double compute_actual_ratio() const {
        return double(m_pixel_height / m_pixel_width);
    }

    void validate() {
        if (m_aspect_ratio <= 0 || m_pixel_width <= 0 || m_samples_per_pixel <= 0 || m_max_depth <= 0) {
            //Error
        }

        m_pixel_height = compute_pixel_height();
        m_sample_scale = compute_sample_scale();
        m_actual_ratio = compute_actual_ratio();
    }
};

#endif //BENDERER_IMAGE_INFO_H