#pragma once

#include <cstdint>

#include <helper/math.hpp>

namespace rasterizer
{
    struct color4ub
    {
        std::uint8_t r, g, b, a;
    };

    struct vector4f
    {
        float x, y, z, w;
    };

    inline color4ub to_color4ub(vector4f const &vect4)
    {
        color4ub result;
        result.r = static_cast<std::uint8_t>(clamp(vect4.x * 255.0f, 0.0f, 255.0f));
        result.g = static_cast<std::uint8_t>(clamp(vect4.y * 255.0f, 0.0f, 255.0f));
        result.b = static_cast<std::uint8_t>(clamp(vect4.z * 255.0f, 0.0f, 255.0f));
        result.a = static_cast<std::uint8_t>(clamp(vect4.w * 255.0f, 0.0f, 255.0f));
        return result;
    }
}
