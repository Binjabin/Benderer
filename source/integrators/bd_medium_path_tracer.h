//
// Created by binjabin on 2/27/26.
//

#ifndef BENDERER_BD_MEDIUM_PATH_TRACER_H
#define BENDERER_BD_MEDIUM_PATH_TRACER_H

#include "../records/medium_scatter_rec.h"
#include "../records/path.h"
#include "../scene/scene.h"
#include "../scene/material/surface_material.h"
#include "../records/path_result.h"
#include "../records/path_state.h"
#include "../samplers/direct_light_sampler.h"
#include "../samplers/medium_sampler.h"

class bd_medium_path_tracer : public integrator {
public:
    bd_medium_path_tracer(int cam_path_length, int light_path_length)
        : m_cam_path_length(cam_path_length), m_light_path_length(light_path_length) {
    }

    path_result path_trace(const ray& r, const world& world, path_state& p_state) const {




        //TODO: INITIALIZE CAM PATH WITH JUST START POS
        if (m_cam_path_length > 1) {
            path_trace_cam();
        }

        //TODO: INITIALIZE EMPTY LIGHT PATH
        if (m_light_path_length > 0) {
            path_trace_light();
        }

        //TODO: Try linking each combination!
        for (int c_i = 0; c_i <= m_cam_path_length; c_i++) {
            for (int l_i = 0; l_i <= m_light_path_length; l_i++) {

                //TODO: Get corresponding path vertecies
                //TODO: Check paths can connect in a valid way...
                //TODO: Calculate chance of that ray being chosen
                //TODO: Calculate and add contribution
            }
        }

        return path_result::color_path_result(colors::black);
    }

private:
    const int m_cam_path_length;
    const int m_light_path_length;

    void path_trace_cam(const ray& cam_ray, const world& world) const {
        path cam_path = path(m_cam_path_length);

        //Store camera vertex
        path_vertex cam_vertex;
        cam_vertex.m_type = vertex_type::Camera;
        cam_vertex.m_interaction.m_p = cam_ray.m_origin;
        cam_vertex.m_interaction.m_t = 0.0;
        cam_vertex.m_interaction.m_time = cam_ray.time();

        cam_path.add(std::move(cam_vertex));

        while (path_length < m_cam_path_length) {
            //TODO: SAMPLE DIRECTION FROM CURRENT VERTEX
            //TODO: SEND RAY INTO SCENE
            //TODO: CALCULATE IMPORTANT VALUES, STORE VERTEX IN PATH
            path_length++;
        }

        //TODO: RETURN PATH
    }

    void path_trace_light(const world& world) const {

        //TODO: SAMPLE POINT ON LIGHT
        int path_length = 1;

        while (path_length < m_light_path_length) {
            //TODO: SAMPLE DIRECTION FROM CURRENT VERTEX
            //TODO: SEND RAY INTO SCENE
            //TODO: CALCULATE IMPORTANT VALUES, STORE VERTEX IN PATH
            path_length++;
        }

        //TODO: RETURN PATH
    }

    bool trace_to_next(const ray& r, const world& world,  path_vertex& result) {

        result = path_vertex();

        //---------------------------------------
        // Check for a surface hit
        surface_hit_rec rec;
        interval ray_t = interval(epsilon, infinity);
        bool hit_surface = world.m_surfaces->surface_hit( r, ray_t, rec );

        //---------------------------------------
        // Check for a medium hits

        double end_t = hit_surface ? rec.get_t() : infinity;
        interval medium_interval = interval(epsilon, end_t);

        //Check for media up to first surface
        medium_intersections medium_recs;
        bool intersect_medium = world.m_media->medium_hit(r, medium_interval, medium_recs);

        medium_hit_rec medium_rec;
        bool hit_medium = false;
        if (intersect_medium) {
            hit_medium = medium_sampler::sample_distance(r, medium_recs, medium_interval, medium_rec);
        }



        if (hit_medium) {

        }

        if (hit_surface) {

        }

        return false;
    }
};




#endif //BENDERER_BD_MEDIUM_PATH_TRACER_H