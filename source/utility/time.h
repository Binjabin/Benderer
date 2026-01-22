//
// Created by binjabin on 1/22/26.
//

#ifndef BENDERER_TIME_H
#define BENDERER_TIME_H

#include <ctime>

inline const char* get_time_string() {
    static char buf[32];
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
    return buf;
}

#endif //BENDERER_TIME_H