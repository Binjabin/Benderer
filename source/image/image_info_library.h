//
// Created by binjabin on 12/7/25.
//

#ifndef BENDERER_IMAGE_INFO_LIBRARY_H
#define BENDERER_IMAGE_INFO_LIBRARY_H
#include "image_info.h"

class image_info_library {
public:
    static image_info micro_low() {
        return image_info(
            1.0,
            64,
            8,
            8
            );
    }

    static image_info micro_ultra() {
        return image_info(
            1.0,
            64,
            128,
            16
            );
    }

    static image_info micro_sol() {
        return image_info(
            1.0,
            64,
            1024,
            64
            );
    }

    static image_info small_low() {
        return image_info(
            1.0,
            320,
            16,
            32
            );
    }

    static image_info small_high() {
        return image_info(
            1.0,
            320,
            64,
            32
            );
    }

    static image_info medium_vlow() {
        return image_info(
        1.0,
        640,
        4,
        8
        );
    }

    static image_info medium_standard() {
        return image_info(
            1.0,
            640,
            32,
            8
            );
    }

    static image_info large_high() {
        return image_info(
            1.0,
            640,
            64,
            16
            );
    }

    static image_info mega_high() {
        return image_info(
            1.0,
            1280,
            64,
            16
            );
    }

    static image_info mega_ultra() {
        return image_info(
            1.0,
            1280,
            128,
            32
            );
    }

    static image_info ablation_1() {
        return image_info(
            1.0,
            256,
            4,
            16
            );
    }

    static image_info ablation_2() {
        return image_info(
            1.0,
            256,
            16,
            16
            );
    }

    static image_info ablation_3() {
        return image_info(
            1.0,
            256,
            64,
            16
            );
    }

    static image_info ablation_4() {
        return image_info(
            1.0,
            256,
            256,
            16
            );
    }

    static image_info ablation_ref() {
        return image_info(
            1.0,
            256,
            4096,
            16
            );
    }

    // High-quality preset for the cover image. 1920px square, 2048 spp and
    // depth 64 — clouds need many scatter events for the bright forward-lit
    // edges to converge. With mis_medium_path_tracer(64, 8, 8) this will be
    // slow but very low noise.
    static image_info cover_hero() {
        return image_info(
            1.0,
            1920,
            2048,
            64
            );
    }

    // Faster look-dev preset — same composition, ~16x cheaper, a bit noisier.
    // Use this to dial in the cloud / sun framing before a full render.
    static image_info cover_lookdev() {
        return image_info(
            1.0,
            960,
            256,
            48
            );
    }

};

#endif //BENDERER_IMAGE_INFO_LIBRARY_H


