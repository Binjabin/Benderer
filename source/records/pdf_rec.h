//
// Created by binjabin on 1/30/26.
//

#ifndef BENDERER_PDF_REC_H
#define BENDERER_PDF_REC_H

class pdf_rec {
public:
    pdf_rec() = default;
    ~pdf_rec() = default;

    vec3 direction;
    double pdf;
};

#endif //BENDERER_PDF_REC_H