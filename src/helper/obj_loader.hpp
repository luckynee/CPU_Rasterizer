#pragma once

#include <rasterizer/types.hpp>

#include <vector>
#include <string>

namespace helper
{
    std::vector<rasterizer::vector3f> load_obj(const std::string &filename);
}