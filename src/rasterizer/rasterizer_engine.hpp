#pragma once

#include "types.hpp"
#include "model.hpp"
#include "types_math.hpp"

namespace rasterizer
{
    class rasterizer_engine
    {
    public:
        int m_width;
        int m_height;
        vector2f m_screen;

        rasterizer_engine(int width, int height)
            : m_width(width),
              m_height(height),
              m_screen(static_cast<float>(width), static_cast<float>(height))
        {
        }

        void draw_to_pixel(
            const model &model,
            std::vector<float> &depth_buffer,
            std::uint32_t *pixels);
    };
}