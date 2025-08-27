#pragma once

#include <helper/image_view.hpp>

namespace rasterizer
{
    void clear(image_view const &color_buffer, vector4f const &color);
}
