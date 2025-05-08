#pragma once

#include<SDL2/SDL.h>
#include "math.hpp" // Cho vector2d
#include <cmath>    // Cho std::sqrt

namespace utils
{
    inline double hireTimeInSeconds() // Giữ double nếu bạn dùng double cho timing
    {
        double t = SDL_GetTicks();
        t *= 0.001; // Sửa 0.001d thành 0.001
        return t;
    }

    // Hàm tính khoảng cách giữa hai vector2d
    inline float distance(const vector2d& p1, const vector2d& p2) {
        float dx = p1.x - p2.x;
        float dy = p1.y - p2.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Hàm clamp giá trị trong một khoảng
    template <typename T>
    inline T clamp(const T& value, const T& min_val, const T& max_val) {
        return std::max(min_val, std::min(value, max_val));
    }
}