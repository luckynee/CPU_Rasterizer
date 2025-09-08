#pragma once

#include <rasterizer/types.hpp>

#include <map>
#include <vector>
#include <string>

namespace helper
{

    // Model

    struct model_data
    {
        rasterizer::mesh_data mesh;
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

    // Texture

    std::vector<std::uint8_t> load_bytes_texture(const std::string &filename);

    rasterizer::texture create_texture_from_bytes(const std::vector<std::uint8_t> &bytes);
}