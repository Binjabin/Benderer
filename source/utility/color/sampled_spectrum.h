//
// Created by binjabin on 2/12/26.
//

#ifndef BENDERER_SAMPLED_SPECTRUM_H
#define BENDERER_SAMPLED_SPECTRUM_H
#include <vector>

#include "spectrum.h"
#include "../../benderer.h"

//This is not a physical spectrum but is the class used in computation as
//The structure to do the actual computations with

constexpr int n_samples_for_spectrum = 5;
class sampled_spectrum {
public:
    //=================
    // Constructors
    //=================

    sampled_spectrum() {
        values.resize(n_samples_for_spectrum, 0.0);
        lambda.resize(n_samples_for_spectrum, 0.0);
        pdf.resize(n_samples_for_spectrum, 0.0);
    }

    sampled_spectrum(double v) {
        values.resize(n_samples_for_spectrum, v);
        lambda.resize(n_samples_for_spectrum, 0.0);
        pdf.resize(n_samples_for_spectrum, 0.0);
    }

    //====================
    // Sample
    //====================

    static sampled_spectrum sample_visible(double u) {
        sampled_spectrum spec;

        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            // Compute up for i-th wavelength sample
            double up = u + static_cast<double>(i) / n_samples_for_spectrum;
            if (up > 1.0)
                up -= 1.0;

            spec.lambda[i] = sample_visible_wavelengths(up);
            spec.pdf[i] = visible_wavelengths_pdf(spec.lambda[i]);
        }
        return spec;
    }

    //=======================
    // Arithmetic operations:
    //=======================
    sampled_spectrum operator+(const sampled_spectrum &other) const {
        sampled_spectrum result = *this;
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            result.values[i] += other.values[i];
        }
        return result;
    }

    sampled_spectrum operator-(const sampled_spectrum &other) const {
        sampled_spectrum result = *this;
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            result.values[i] -= other.values[i];
        }
        return result;
    }

    sampled_spectrum operator*(const sampled_spectrum &other) const {
        sampled_spectrum result = *this;
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            result.values[i] *= other.values[i];
        }
        return result;
    }

    sampled_spectrum operator*(double a) const {
        sampled_spectrum result = *this;
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            result.values[i] *= a;
        }
        return result;
    }

    sampled_spectrum operator/(double a) const {
        sampled_spectrum result = *this;
        double inv_a = 1 / a;
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            result.values[i] *= inv_a;
        }
        return result;
    }

    sampled_spectrum& operator+=(const sampled_spectrum &other) {
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            values[i] += other.values[i];
        }
        return *this;
    }

    sampled_spectrum& operator*=(const sampled_spectrum &other) {
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            values[i] *= other.values[i];
        }
        return *this;
    }

    sampled_spectrum& operator*=(double a) {
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            values[i] *= a;
        }
        return *this;
    }

    //======================
    // Utility
    //======================

    static sampled_spectrum from_spectrum(const spectrum& s, double lambda_sample) {
        sampled_spectrum result = sample_visible(lambda_sample);
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            result.values[i] = s(result.lambda[i]);
        }
        return result;
    }

    bool is_black() const {
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            if (values[i] != 0) return false;
        }
        return true;
    }

    double max_value() const {
        double max_val = values[0];
        for (int i = 1; i < n_samples_for_spectrum; ++i) {
            if (values[i] > max_val) max_val = values[i];
        }
        return max_val;
    }

    double average() const {
        double sum = 0;
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            sum += values[i];
        }
        return sum / n_samples_for_spectrum;
    }

    sampled_spectrum clamp(double low = 0.0, double high = infinity) const {
        sampled_spectrum result = *this;
        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            result.values[i] = std::max(low, std::min(high, values[i]));
        }
        return result;
    }

    void to_xyz(double xyz[3]) const {
        xyz[0] = xyz[1] = xyz[2] = 0.0;

        for (int i = 0; i < n_samples_for_spectrum; ++i) {
            if (pdf[i] == 0.0) continue;

            // Get CIE XYZ color matching function values at this wavelength
            double x_bar = cie_x(lambda[i]);
            double y_bar = cie_y(lambda[i]);
            double z_bar = cie_z(lambda[i]);

            // Weighted sum
            xyz[0] += values[i] * x_bar / pdf[i];
            xyz[1] += values[i] * y_bar / pdf[i];
            xyz[2] += values[i] * z_bar / pdf[i];
        }

        // Normalize
        double scale = double(lambda_max - lambda_min) /
                      (cie_y_integral * n_samples_for_spectrum);
        xyz[0] *= scale;
        xyz[1] *= scale;
        xyz[2] *= scale;
    }

    void to_rgb(double rgb[3]) const {
        double xyz[3];
        to_xyz(xyz);
        xyz_to_rgb(xyz, rgb);
    }

    //=================
    // Access
    //=================

    double get_value(int i) const { return values[i]; }
    double get_lambda(int i) const { return lambda[i]; }
    double get_pdf(int i) const { return pdf[i]; }


private:
    std::vector<double> values;
    std::vector<double> lambda;
    std::vector<double> pdf;


    //=================
    // CIE Approximation
    //=================

    // Helper: Sample visible wavelengths with importance sampling
    static double sample_visible_wavelengths(double u) {
        // TODO: For true importance sampling, invert the CDF of CIE Y
        // Simplified approximation
        return lambda_min + u * (lambda_max - lambda_min);
    }

    static double visible_wavelengths_pdf(double lambda) {
        // PDF for uniform sampling
        if (lambda >= lambda_min && lambda <= lambda_max)
            return 1.0 / (lambda_max - lambda_min);
        return 0.0;
    }

    static double cie_x(double lambda) {
        double t1 = (lambda - 442.0) * (lambda < 442.0 ? 0.0624 : 0.0374);
        double t2 = (lambda - 599.8) * (lambda < 599.8 ? 0.0264 : 0.0323);
        double t3 = (lambda - 501.1) * (lambda < 501.1 ? 0.0490 : 0.0382);
        return 0.362 * std::exp(-0.5 * t1 * t1) +
               1.056 * std::exp(-0.5 * t2 * t2) -
               0.065 * std::exp(-0.5 * t3 * t3);
    }

    static double cie_y(double lambda) {
        double t = (lambda - 556.1) * (lambda < 556.1 ? 0.0213 : 0.0247);
        return 0.821 * std::exp(-0.5 * t * t) + 0.286;
    }

    static double cie_z(double lambda) {
        double t = (lambda - 437.0) * (lambda < 437.0 ? 0.0845 : 0.0278);
        return 1.217 * std::exp(-0.5 * t * t);
    }

    static void xyz_to_rgb(const double xyz[3], double rgb[3]) {
        rgb[0] =  3.240479 * xyz[0] - 1.537150 * xyz[1] - 0.498535 * xyz[2];
        rgb[1] = -0.969256 * xyz[0] + 1.875991 * xyz[1] + 0.041556 * xyz[2];
        rgb[2] =  0.055648 * xyz[0] - 0.204043 * xyz[1] + 1.057311 * xyz[2];
    }

    static constexpr double cie_y_integral = 106.856895;
};



#endif //BENDERER_SAMPLED_SPECTRUM_H