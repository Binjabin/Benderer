//
// Created by binjabin on 12/7/25.
//

#ifndef BENDERER_IMAGE_INFO_LIBRARY_H
#define BENDERER_IMAGE_INFO_LIBRARY_H
#include "image_info.h"

#endif //BENDERER_IMAGE_INFO_LIBRARY_H

class image_info_library {
public:
    static image_info preview() {
        return image_info(
            1.0,
            64,
            8,
            2
            );
    }

    static image_info low() {
        return image_info(
            1.0,
            640,
            32,
            4
            );
    }

    static image_info medium() {
        return image_info(
            1.0,
            640,
            64,
            8
            );
    }

    static image_info high() {
        return image_info(
            1.0,
            1280,
            64,
            8
            );
    }

    static image_info ultra() {
        return image_info(
            1.0,
            1280,
            128,
            16
            );
    }
};




