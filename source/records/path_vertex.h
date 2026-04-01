//
// Created by binjabin on 2/27/26.
//

#ifndef BENDERER_PATH_VERTEX_H
#define BENDERER_PATH_VERTEX_H
#include "interaction.h"
#include "../scene/material/surface_material.h"
#include "../scene/material/medium_material.h"

//Camera + Light are the start and end vertecies
enum class vertex_type { Camera, Light, Surface, Medium };

struct path_vertex {

    vertex_type m_type = vertex_type::Surface;
    interaction m_interaction;

    color m_emission = colors::black;

    color local_throughput = colors::white;

    bool m_is_delta = false;

    //Direction towards previous vertex.
    vec3 m_wi = uninit_vec;

    double m_pdf_w_forward = uninit;
    double m_pdf_w_backward = uninit;

    //Only set for surface vertices
    vec3 m_normal = uninit_vec;
    shared_ptr<surface_material> m_surface_mat = nullptr;

    //Only set for medium vertices
    shared_ptr<medium_material> m_medium_mat = nullptr;
};

#endif //BENDERER_PATH_VERTEX_H