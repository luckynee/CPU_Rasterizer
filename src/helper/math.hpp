#pragma once

#include "rasterizer/types.hpp"

namespace math
{
    constexpr float PI = 3.14159265358979323846f;

    template <typename T>
    constexpr T min(const T &a, const T &b) { return (a < b) ? a : b; }

    template <typename T>
    constexpr T max(const T &a, const T &b) { return (a > b) ? a : b; }

    template <typename T>
    constexpr T clamp(const T &value, const T &minVal, const T &maxVal)
    {
        return max(minVal, min(value, maxVal));
    }

    template <typename T>
    constexpr T floor(const T &value)
    {
        return static_cast<T>(static_cast<int>(value) - (value < static_cast<T>(0) && value != static_cast<T>(static_cast<int>(value))));
    }

    template <typename T>
    constexpr T ceil(const T &value)
    {
        return static_cast<T>(static_cast<int>(value) + (value > static_cast<T>(0) && value != static_cast<T>(static_cast<int>(value))));
    }

    // Normalize angle to [-PI, PI]
    constexpr float wrap_pi(float x)
    {
        while (x < -PI)
            x += 2 * PI;
        while (x > PI)
            x -= 2 * PI;
        return x;
    }

    // Sine approximation (Taylor series, accurate for small |x|)
    constexpr float sin(float x)
    {
        x = wrap_pi(x);
        float x2 = x * x;
        return x - (x2 * x) / 6.0f + (x2 * x2 * x) / 120.0f - (x2 * x2 * x2 * x) / 5040.0f;
    }

    // Cosine approximation (Taylor series)
    constexpr float cos(float x)
    {
        x = wrap_pi(x);
        float x2 = x * x;
        return 1.0f - x2 / 2.0f + (x2 * x2) / 24.0f - (x2 * x2 * x2) / 720.0f;
    }

    // Tangent as sin/cos
    constexpr float tan(float x)
    {
        return sin(x) / cos(x);
    }

    constexpr float atan(float x)
    {
        // Abramowitz and Stegun formula 4.4.40
        // Accurate for |x| <= 1
        float abs_x = x < 0 ? -x : x;
        float result;
        if (abs_x <= 1.0f)
        {
            result = x / (1.0f + 0.28f * x * x);
        }
        else
        {
            result = (PI / 2) - x / (x * x + 0.28f);
            if (x < 0)
                result = -result;
        }
        return result;
    }

    template <typename T>
    constexpr T abs(const T &value)
    {
        return (value < 0) ? -value : value;
    }

    constexpr float to_radians(float degrees)
    {
        return degrees * (PI / 180.0f);
    }

    constexpr float sqrt(float x)
    {
        if (x < 0.0f)
            return 0.0f; // or handle error as needed
        if (x == 0.0f)
            return 0.0f;

        float guess = x;
        for (int i = 0; i < 10; ++i) // 10 iterations is usually enough
        {
            guess = 0.5f * (guess + x / guess);
        }
        return guess;
    }
}
