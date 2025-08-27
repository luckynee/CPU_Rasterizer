#pragma once

namespace rasterizer
{
    template <typename T>
    constexpr T min(const T &a, const T &b) { return (a < b) ? a : b; }

    template <typename T>
    constexpr T max(const T &a, const T &b) { return (a > b) ? a : b; }

    template <typename T>
    constexpr T clamp(const T &value, const T &minVal, const T &maxVal)
    {
        return max(minVal, min(value, maxVal));
    }

}
