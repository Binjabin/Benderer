//
// Created by binjabin on 12/7/25.
//

#ifndef BENDERER_IMAGE_INFO_LIBRARY_H
#define BENDERER_IMAGE_INFO_LIBRARY_H
#include "image_info.h"

#endif //BENDERER_IMAGE_INFO_LIBRARY_H

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
};




