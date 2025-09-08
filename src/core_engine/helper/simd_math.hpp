#pragma once

#include <xmmintrin.h>

#include "rasterizer/types.hpp"

namespace helper
{
    inline __m128 calculate_barycentric_areas_simd(
        const rasterizer::vector2f &a,
        const rasterizer::vector2f &b,
        const rasterizer::vector2f &c,
        const rasterizer::vector2f &p)
    {

        // --- Data Setup ---
        // The _mm_set_ps intrinsic loads four floats into a 128-bit SIMD register.
        // The argument order is reversed (w, z, y, x). We are setting up "lanes"
        // to calculate the three areas (pbc, apc, abp) at the same time.

        // Load the X coordinates of the first vertex for each area calculation.
        // Lane 0: p.x (for pbc), Lane 1: a.x (for apc), Lane 2: a.x (for abp)
        __m128 p1_x = _mm_set_ps(0.0f, a.x, a.x, p.x);
        // Load the Y coordinates of the first vertex for each area calculation.
        __m128 p1_y = _mm_set_ps(0.0f, a.y, a.y, p.y);

        // Load the X coords for the second vertex of each area.
        // Lane 0: b.x, Lane 1: p.x, Lane 2: b.x
        __m128 p2_x = _mm_set_ps(0.0f, b.x, p.x, b.x);
        // Load the Y coords for the second vertex of each area.
        __m128 p2_y = _mm_set_ps(0.0f, b.y, p.y, b.y);

        // Load the X coords for the third vertex of each area.
        // Lane 0: c.x, Lane 1: c.x, Lane 2: p.x
        __m128 p3_x = _mm_set_ps(0.0f, p.x, c.x, c.x);
        // Load the Y coords for the third vertex of each area.
        __m128 p3_y = _mm_set_ps(0.0f, p.y, c.y, c.y);

        // --- SIMD Calculation ---
        // Perform subtractions on all four lanes at once to get the edge vectors.
        // This calculates vectors like (c - p), (c - a), and (p - a) all at the same time.
        __m128 v21_x = _mm_sub_ps(p3_x, p1_x); // Vector from p1 to p3 (x components)
        __m128 v21_y = _mm_sub_ps(p3_y, p1_y); // Vector from p1 to p3 (y components)
        __m128 v10_x = _mm_sub_ps(p2_x, p1_x); // Vector from p1 to p2 (x components)
        __m128 v10_y = _mm_sub_ps(p2_y, p1_y); // Vector from p1 to p2 (y components)

        // Perform multiplications on all four lanes at once.
        __m128 term1 = _mm_mul_ps(v21_x, v10_y);
        __m128 term2 = _mm_mul_ps(v21_y, v10_x);

        // Perform the final subtraction to get the 2D cross-product (which is 2 * area).
        // This single instruction finalizes the calculation for all three areas.
        return _mm_sub_ps(term1, term2);
    }

}