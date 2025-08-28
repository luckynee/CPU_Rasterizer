#pragma once

#include <rasterizer/types.hpp>

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

}
