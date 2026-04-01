//
// Created by binjabin on 12/26/25.
//

#ifndef BENDERER_PATH_RESULT_H
#define BENDERER_PATH_RESULT_H

struct path_result {
    color radiance_from_path;
    bool terminated_on_light;

    static path_result color_path_result(color radiance) {
        path_result res = empty_path_result();
        res.radiance_from_path = radiance;
        return res;
    }

    static path_result empty_path_result() {
        path_result out_result;
        out_result.radiance_from_path = color(0,0,0);
        out_result.terminated_on_light = false;
        return out_result;
    }
};

#endif //BENDERER_PATH_RESULT_H