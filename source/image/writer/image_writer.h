//
// Created by binjabin on 1/22/26.
//

#ifndef BENDERER_IMAGE_MAKER_H
#define BENDERER_IMAGE_MAKER_H
#include "../post/post_process.h"

class image_writer {
public:
    virtual ~image_writer() = default;
    virtual void write(const char* filename, const std::vector<double>& double_buf) const = 0;
};

#endif //BENDERER_IMAGE_MAKER_H