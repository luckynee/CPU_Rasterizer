#pragma once

#include "types.hpp"

namespace rasterizer
{
    //
    // Vector Math Functions
    //

    inline float dot(const vector2f &a, const vector2f &b)
    {
        return a.x * b.x + a.y * b.y;
    }

    inline float dot(const vector3f &a, const vector3f &b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

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

    inline vector2f perpendicular(const vector2f &v)
    {
        return vector2f{v.y, -v.x};
    }

    //
    // Triangle Functions
    //

    inline float signed_triangle_area(const vector2f &a, const vector2f &b, const vector2f &c)
    {
        // using float instead of vector2f for p to avoid unnecessary struct construction
        float acx = c.x - a.x;
        float acy = c.y - a.y;

        float abx = b.x - a.x;
        float aby = b.y - a.y;

        float ab_perp_x = aby;
        float ab_perp_y = -abx;

        float dot = acx * ab_perp_x + acy * ab_perp_y;

        return dot / 2.0f;
    }

    inline bool point_in_triangle(
        const vector2f &a,
        const vector2f &b,
        const vector2f &c,
        float px,
        float py,
        vector3f &weights)
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

        return weights.x >= 0 && weights.y >= 0 && weights.z >= 0;
    }

}