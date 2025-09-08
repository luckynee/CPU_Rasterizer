#pragma once

namespace math
{
    //
    // Constants
    //
    constexpr float PI = 3.14159265358979323846f;

    //
    // Min/Max/Clamp
    //
    template <typename T>
    constexpr T min(const T &a, const T &b)
    {
        return (a < b) ? a : b;
    }

    template <typename T>
    constexpr T max(const T &a, const T &b)
    {
        return (a > b) ? a : b;
    }

    template <typename T>
    constexpr T clamp(const T &value, const T &minVal, const T &maxVal)
    {
        return max(minVal, min(value, maxVal));
    }

    //
    // Rounding
    //

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

    //
    // Angle and Trigonometry
    //
    constexpr float wrap_pi(float x)
    {
        while (x < -PI)
            x += 2 * PI;
        while (x > PI)
            x -= 2 * PI;
        return x;
    }

    constexpr float sin(float x)
    {
        x = wrap_pi(x);
        float x2 = x * x;
        float x3 = x2 * x;
        float x5 = x3 * x2;
        float x7 = x5 * x2;
        float x9 = x7 * x2;
        return x - x3 / 6.0f + x5 / 120.0f - x7 / 5040.0f + x9 / 362880.0f;
    }

    constexpr float cos(float x)
    {
        x = wrap_pi(x);
        float x2 = x * x;
        float x4 = x2 * x2;
        float x6 = x4 * x2;
        float x8 = x4 * x4;
        float x10 = x8 * x2;
        return 1.0f - x2 / 2.0f + x4 / 24.0f - x6 / 720.0f + x8 / 40320.0f - x10 / 3628800.0f;
    }

    constexpr float tan(float x)
    {
        return sin(x) / cos(x);
    }

    constexpr float atan(float x)
    {
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

    constexpr float to_radians(float degrees)
    {
        return degrees * (PI / 180.0f);
    }

    //
    // Absolute Value , Square Root, Modulus, Power, Exponential
    //

    template <typename T>
    constexpr T abs(const T &value)
    {
        return (value < 0) ? -value : value;
    }

    constexpr float sqrt(float x)
    {
        if (x < 0.0f)
            return 0.0f;
        if (x == 0.0f)
            return 0.0f;

        float guess = x;
        for (int i = 0; i < 10; ++i)
        {
            guess = 0.5f * (guess + x / guess);
        }

        return guess;
    }

    constexpr float fmod(float x, float y)
    {
        if (y == 0.0f)
            return 0.0f;

        return x - y * static_cast<int>(x / y);
    }

    constexpr float exp(float x)
    {
        float sum = 1.0f;
        float term = 1.0f;
        for (int i = 1; i <= 6; ++i)
        {
            term *= x / i;
            sum += term;
        }
        return sum;
    }

    constexpr float log(float x)
    {
        if (x <= 0.0f)
            return 0.0f;

        float y = (x - 1.0f) / (x + 1.0f);
        float y2 = y * y;
        return 2.0f * (y + y2 * y / 3.0f + y2 * y2 * y / 5.0f);
    }

    template <typename T>
    constexpr T pow(const T &base, int exp)
    {
        T result = static_cast<T>(1);
        int abs_exp = (exp < 0) ? -exp : exp;
        for (int i = 0; i < abs_exp; ++i)
            result *= base;

        return (exp < 0) ? static_cast<T>(1) / result : result;
    }

    template <typename T>
    inline T pow(const T &base, float exp)
    {
        return math::exp(exp * math::log(base));
    }

    //
    // Interpolation
    //

    template <typename T>
    inline T lerp(const T &a, const T &b, float t)
    {
        t = math::clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }
}
