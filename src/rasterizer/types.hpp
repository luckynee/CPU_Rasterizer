#pragma once

#include <cstdint>

#include <rasterizer/math.hpp>

namespace rasterizer
{
    struct vector2f
    {
        float x, y;

        vector2f operator-(const vector2f &other) const
        {
            return vector2f{x - other.x, y - other.y};
        }

        vector2f operator+(const vector2f &other) const
        {
            return vector2f{x + other.x, y + other.y};
        }

        vector2f operator*(float scalar) const
        {
            return vector2f{x * scalar, y * scalar};
        }

        vector2f operator/(float scalar) const
        {
            return vector2f{x / scalar, y / scalar};
        }

        vector2f &operator+=(const vector2f &other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }
    };

    struct vector3f
    {
        float x, y, z;

        vector3f operator-(const vector3f &other) const
        {
            return vector3f{x - other.x, y - other.y, z - other.z};
        }

        vector3f operator+(const vector3f &other) const
        {
            return vector3f{x + other.x, y + other.y, z + other.z};
        }
    };

    struct vector4f
    {
        float x, y, z, w;
    };

    struct color4ub
    {
        std::uint8_t r, g, b, a;
    };

    struct Random
    {
        std::uint32_t state;

        Random(std::uint32_t seed) : state(seed)
        {
            if (state == 0)
                state = 1; // Avoid zero state
        }

        std::uint32_t next()
        {
            // Xorshift algorithm
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return state;
        }

        // Random float between 0.0 and 1.0
        float next_float()
        {
            return static_cast<float>(next()) / static_cast<float>(UINT32_MAX);
        }

        // Random float in range [min, max]
        float next_float(float min, float max)
        {
            return min + next_float() * (max - min);
        }

        vector2f next_vector2f(float min = 0.0f, float max = 1.0f)
        {
            return vector2f{next_float(min, max), next_float(min, max)};
        }

        vector3f next_vector3f(float min = 0.0f, float max = 1.0f)
        {
            return vector3f{next_float(min, max), next_float(min, max), next_float(min, max)};
        }
    };

    //
    // Casting Functionshow to create
    //

    inline color4ub
    to_color4ub(const vector4f &vec)
    {
        return color4ub{
            static_cast<std::uint8_t>(clamp(vec.x * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(clamp(vec.y * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(clamp(vec.z * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(clamp(vec.w * 255.0f, 0.0f, 255.0f))};
    }

    inline color4ub to_color4ub(const vector3f &vec)
    {
        return color4ub{
            static_cast<std::uint8_t>(clamp(vec.x * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(clamp(vec.y * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(clamp(vec.z * 255.0f, 0.0f, 255.0f)),
            255};
    }

    inline std::uint32_t to_uint32(const color4ub &col)
    {
        // For SDL_PIXELFORMAT_RGBA32, using ABGR order
        return (static_cast<std::uint32_t>(col.a) << 24) |
               (static_cast<std::uint32_t>(col.b) << 16) |
               (static_cast<std::uint32_t>(col.g) << 8) |
               (static_cast<std::uint32_t>(col.r));
    }

    //
    // Calculations
    //

    // Calculate Dot Product of a and b
    // Product of their lengths, times the cosine of the angle between them
    inline float dot(const vector2f &a, const vector2f &b) { return a.x * b.x + a.y * b.y; }
    inline float dot(const vector3f &a, const vector3f &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

    // Calculate the perpendicular vector ( 90 degress clockwise from given vector)
    inline vector2f perpendicular(const vector2f &v) { return vector2f{v.y, -v.x}; }

    // Check if the point on the right side of line
    inline bool point_on_right_side(const vector2f &a, const vector2f &b, const vector2f &p)
    {
        vector2f ap = p - a;
        vector2f abPerp = perpendicular(b - a);
        return dot(abPerp, ap) >= 0;
    }

    // Check if the specific point inside the triangle
    inline bool point_in_triangle(const vector2f &a, const vector2f &b, const vector2f &c, const vector2f &p)
    {
        bool sideAB = point_on_right_side(a, b, p);
        bool sideBC = point_on_right_side(b, c, p);
        bool sideCA = point_on_right_side(c, a, p);
        return sideAB && sideBC && sideBC && sideCA;
    }
}
