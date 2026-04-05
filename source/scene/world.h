//
// Created by binjabin on 1/17/26.
//

#ifndef BENDERER_WORLD_H
#define BENDERER_WORLD_H

#include "hittables/hittable.h"
#include "hittables/mediums/medium.h"
#include "hittables/mediums/medium_list.h"
#include "hittables/mediums/medium_tree.h"
#include "hittables/surfaces/surface_list.h"
#include "hittables/surfaces/surface_tree.h"
#include "skyboxes/skybox.h"

class world {
public:
    ~world() = default;

    world( shared_ptr<surface> surfaces, shared_ptr<medium> media, shared_ptr<surface> surface_lights, shared_ptr<medium> volume_lights, const shared_ptr<skybox> skybox)
        : m_surfaces( surfaces ), m_media(media), m_surface_lights( surface_lights ), m_volume_lights(volume_lights), m_sky( skybox ),
        m_furthest_distance(std::max(media->global_furthest_point(), surfaces->global_furthest_point())) {
    }

    void accelerate() {
        m_surfaces = make_shared<surface_tree_node>(m_surfaces);
        m_media = make_shared<medium_tree_node>(m_media);
    }

    shared_ptr<surface> m_surfaces;
    shared_ptr<medium> m_media;
    shared_ptr<surface> m_surface_lights;
    shared_ptr<medium> m_volume_lights;

    const shared_ptr<skybox> m_sky;

    const double m_furthest_distance;
};

#endif //BENDERER_WORLD_H