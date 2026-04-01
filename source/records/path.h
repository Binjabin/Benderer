//
// Created by binjabin on 2/27/26.
//

#ifndef BENDERER_PATH_H
#define BENDERER_PATH_H
#include <vector>

#include "path_vertex.h"

class path {

public:
    path(int max_length, bool forward)
        : m_forward(forward) {
        m_vertices.reserve(max_length);
    }

    const path_vertex& operator[](int i) const { return m_vertices[i]; }

    path_vertex& operator[](const int i) {
        return m_vertices[i];
    }

    void add(path_vertex&& v) {
        m_vertices.emplace_back(std::move(v));
        if (!v.m_is_delta) m_path_sample_depth++;
    }

private:
    std::vector<path_vertex> m_vertices;
    //Forward, ie from camera, or backwards, ie from light
    bool m_forward = true;

};

#endif //BENDERER_PATH_H