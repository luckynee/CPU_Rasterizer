#pragma once

#include <rasterizer/types.hpp>

#include <map>
#include <vector>
#include <string>

namespace helper
{

    struct model_data
    {
        std::vector<rasterizer::vertex_data> vertices;
        std::vector<std::uint32_t> indices;
    };

    unsigned int parse_face_vertex(
        const std::string &vertex_string,
        const std::vector<rasterizer::vector3f> &temp_pos,
        const std::vector<rasterizer::vector2f> &temp_tex,
        const std::vector<rasterizer::vector3f> &temp_norm,
        std::map<std::string, unsigned int> &known_vertices,
        model_data &out_model_data);

    model_data load_obj(const std::string &filename);

    std::vector<std::uint8_t> load_bytes_texture(const std::string &filename);
}