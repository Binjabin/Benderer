//
// Created by binjabin on 12/7/25.
//


#ifndef BENDERER_SCATTER_RECORD_H
#define BENDERER_SCATTER_RECORD_H

#include "../structures/pdf.h"

class scatter_record {
public:
    color attenuation;
    shared_ptr<pdf> pdf_ptr;
    bool skip_pdf;
    ray skip_pdf_ray;
};

#endif //BENDERER_SCATTER_RECORD_H