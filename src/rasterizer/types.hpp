#pragma once

#include <cstdint>

#include "helper/math.hpp"

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

        void reset_to_zero()
        {
            x = 0;
            y = 0;
            z = 0;
        }

        vector3f operator-(const vector3f &other) const
        {
            return vector3f{x - other.x, y - other.y, z - other.z};
        }

        vector3f operator+(const vector3f &other) const
        {
            return vector3f{x + other.x, y + other.y, z + other.z};
        }

        vector3f operator*(float scalar) const
        {
            return vector3f{x * scalar, y * scalar, z * scalar};
        }

        vector3f operator/(float scalar) const
        {
            return vector3f{x / scalar, y / scalar, z / scalar};
        }

        vector3f operator+=(const vector3f &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        vector3f operator-=(const vector3f &other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        explicit operator vector2f() const
        {
            return vector2f{x, y};
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
            static_cast<std::uint8_t>(math::clamp(vec.x * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(math::clamp(vec.y * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(math::clamp(vec.z * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(math::clamp(vec.w * 255.0f, 0.0f, 255.0f))};
    }

    inline color4ub to_color4ub(const vector3f &vec)
    {
        return color4ub{
            static_cast<std::uint8_t>(math::clamp(vec.x * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(math::clamp(vec.y * 255.0f, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(math::clamp(vec.z * 255.0f, 0.0f, 255.0f)),
            255};
    }

    // Converts vector3f (r,g,b in [0,1]) to packed uint32_t RGBA (alpha=255)
    inline uint32_t to_uint32(const rasterizer::vector3f &v)
    {
        uint8_t r = static_cast<uint8_t>(math::clamp(v.x, 0.0f, 1.0f) * 255.0f);
        uint8_t g = static_cast<uint8_t>(math::clamp(v.y, 0.0f, 1.0f) * 255.0f);
        uint8_t b = static_cast<uint8_t>(math::clamp(v.z, 0.0f, 1.0f) * 255.0f);
        uint8_t a = 255;
        return (a << 24) | (r << 16) | (g << 8) | b;
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

    inline vector2f normalized_vector(const vector2f &v)
    {
        float len = math::sqrt(dot(v, v));
        if (len > 1e-6f)
            return v / len;
        return vector2f{0, 0};
    }

    inline vector3f normalized_vector(const vector3f &v)
    {
        float len = math::sqrt(dot(v, v));
        if (len > 1e-6f)
            return v / len;
        return vector3f{0, 0, 0};
    }

    // Calculate the perpendicular vector ( 90 degress clockwise from given vector)
    inline vector2f perpendicular(const vector2f &v) { return vector2f{v.y, -v.x}; }

    // Calculate area of triangle ABC (positive if clockwise, otherwise negative)
    inline float signed_triangle_area(const vector2f &a, const vector2f &b, const vector2f &c)
    {
        float acx = c.x - a.x;
        float acy = c.y - a.y;

        float abx = b.x - a.x;
        float aby = b.y - a.y;

        // perpendicular(b - a)
        float ab_perp_x = aby;
        float ab_perp_y = -abx;

        // dot(ac, ab_perp)
        float dot = acx * ab_perp_x + acy * ab_perp_y;

        return dot / 2.0f;
    }

    // Check if the specific point inside the triangle
    inline bool point_in_triangle(const vector2f &a, const vector2f &b, const vector2f &c, float px, float py, vector3f &weights, float denom)
    {

        // float areaABP = signed_triangle_area(a, b, p);
        // float areaBCP = signed_triangle_area(b, c, p);
        // float areaCAP = signed_triangle_area(c, a, p);
        // bool in_triangle = areaABP >= 0 && areaBCP >= 0 && areaCAP >= 0;

        // // Weighting factors (barycentric coordinates)
        // float total_area = (areaABP + areaBCP + areaCAP);
        // float inv_area_sum = 1 / total_area;
        // float weightAB = areaBCP * inv_area_sum;
        // float weightBC = areaCAP * inv_area_sum;
        // float weightCA = areaABP * inv_area_sum;
        // weights = vector3f{weightAB, weightBC, weightCA};

        // return in_triangle && total_area > 0;

        if (math::abs(denom) < 1e-6f)
            return false; // Degenerate triangle

        float w0 = ((b.y - c.y) * (px - c.x) + (c.x - b.x) * (py - c.y)) / denom;
        float w1 = ((c.y - a.y) * (px - c.x) + (a.x - c.x) * (py - c.y)) / denom;
        float w2 = 1.0f - w0 - w1;

        constexpr float epsilon = -1e-4f;
        bool in_triangle = (w0 > epsilon) && (w1 > epsilon) && (w2 > epsilon);

        weights = vector3f{w0, w1, w2};
        return in_triangle;
    }
}
