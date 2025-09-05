#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>

#include "helper/math.hpp"

namespace rasterizer
{
    struct vector2f
    {
        float x = 0.0f, y = 0.0f;

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
        float x = 0.0f, y = 0.0f, z = 0.0f;

        void reset_to_zero()
        {
            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
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

        vector3f operator*=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
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

    struct vertex_data
    {
        vector3f position;
        vector2f tex_coord;
        vector3f normal;
    };

    struct rasterizer_data
    {
        vector2f position;
        vector2f tex_coord;
        vector3f normal;
        float depth;
    };

    struct texture
    {
        const int width;
        const int height;

        inline texture(int width, int height, std::vector<vector3f> image_data)
            : width(width),
              height(height),
              wscale(width - 1),
              hscale(height - 1),
              m_image_data(std::move(image_data))
        {
        }

        inline vector3f sample_texture(float u, float v) const
        {
            float wrapped_u = math::fmod(u, 1.0f);
            float wrapped_v = math::fmod(v, 1.0f);
            if (wrapped_u < 0)
                wrapped_u += 1.0f;
            if (wrapped_v < 0)
                wrapped_v += 1.0f;

            int x = static_cast<int>(wrapped_u * wscale);
            int y = static_cast<int>(wrapped_v * hscale);

            return m_image_data[y * width + x];
        }

    private:
        const int wscale;
        const int hscale;
        std::vector<vector3f> m_image_data;
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

    inline float lerp(float a, float b, float t)
    {
        t = math::clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }

    inline vector2f lerp(const vector2f &a, const vector2f &b, float t)
    {
        t = math::clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }

    inline vector3f lerp(const vector3f &a, const vector3f &b, float t)
    {
        t = math::clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }

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
    inline bool point_in_triangle(const vector2f &a, const vector2f &b, const vector2f &c, float px, float py, vector3f &weights)
    {
        vector2f p{px, py};
        float area_abc = signed_triangle_area(a, b, c);
        if (area_abc == 0)
            return false;

        float area_pbc = signed_triangle_area(p, b, c);
        float area_apc = signed_triangle_area(a, p, c);
        float area_abp = signed_triangle_area(a, b, p);

        weights.x = area_pbc / area_abc;
        weights.y = area_apc / area_abc;
        weights.z = area_abp / area_abc;

        // Strict inside test
        return weights.x >= 0 && weights.y >= 0 && weights.z >= 0;
    }

    inline texture create_texture_from_bytes(const std::vector<std::uint8_t> &bytes)
    {
        if (bytes.size() < 4)
            throw std::runtime_error("Invalid texture bytes");

        int width = bytes[0] | (bytes[1] << 8);
        int height = bytes[2] | (bytes[3] << 8);

        if (width <= 0 || height <= 0)
            throw std::runtime_error("Invalid texture bytes");

        std::size_t expected_size = 4 + (static_cast<std::size_t>(width) * height * 3);
        if (bytes.size() < expected_size)
            throw std::runtime_error("Invalid texture bytes");

        std::vector<vector3f> image_data(width * height);
        std::size_t byte_index = 4;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                float b = bytes[byte_index + 0] / 255.0f;
                float r = bytes[byte_index + 1] / 255.0f;
                float g = bytes[byte_index + 2] / 255.0f;

                image_data[y * width + x] = vector3f{r, g, b};
                byte_index += 3;
            }
        }

        return texture(width, height, std::move(image_data));
    }

}
