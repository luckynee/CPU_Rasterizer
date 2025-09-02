#include <sstream>
#include <fstream>

#include "helper/obj_loader.hpp"

namespace helper
{
    // TODO-> Fix obj loader implementation
    // ...existing code...
    unsigned int parse_face_vertex(
        const std::string &vertex_string,
        const std::vector<rasterizer::vector3f> &temp_pos,
        const std::vector<rasterizer::vector2f> &temp_tex,
        const std::vector<rasterizer::vector3f> &temp_norm,
        std::map<std::string, unsigned int> &known_vertices,
        model_data &out_model_data)
    {
        if (known_vertices.count(vertex_string))
        {
            return known_vertices[vertex_string];
        }

        std::stringstream ss(vertex_string);
        std::string part;

        unsigned int pos_index = 0, tex_index = 0, norm_index = 0;

        std::getline(ss, part, '/');
        if (!part.empty())
            pos_index = std::stoul(part);

        std::getline(ss, part, '/');
        if (!part.empty())
            tex_index = std::stoul(part);

        std::getline(ss, part, '/');
        if (!part.empty())
            norm_index = std::stoul(part);

        rasterizer::vertex_data new_vertex;
        if (pos_index > 0)
            new_vertex.position = temp_pos[pos_index - 1];
        if (tex_index > 0)
            new_vertex.tex_coord = temp_tex[tex_index - 1];
        if (norm_index > 0)
            new_vertex.normal = temp_norm[norm_index - 1];

        out_model_data.vertices.push_back(new_vertex);
        unsigned int new_index = out_model_data.vertices.size() - 1;
        known_vertices[vertex_string] = new_index;
        return new_index;
    }

    model_data load_obj(const std::string &filename)
    {
        std::vector<rasterizer::vector3f> temp_pos;
        std::vector<rasterizer::vector2f> temp_tex;
        std::vector<rasterizer::vector3f> temp_norm;

        model_data model;
        std::map<std::string, unsigned int> known_vertices;

        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            // Vertex Position
            if (prefix == "v")
            {
                rasterizer::vector3f vertex;
                iss >> vertex.x >> vertex.y >> vertex.z;
                temp_pos.push_back(vertex);
            }
            // Text coords
            else if (prefix == "vt")
            {
                rasterizer::vector2f tex_coord;
                iss >> tex_coord.x >> tex_coord.y;
                temp_tex.push_back(tex_coord);
            }
            // Vertex Normal
            else if (prefix == "vn")
            {
                rasterizer::vector3f normal;
                iss >> normal.x >> normal.y >> normal.z;
                temp_norm.push_back(normal);
            }
            else if (prefix == "f")
            {
                std::vector<unsigned int> face_indices;
                std::string vertex_str;

                while (iss >> vertex_str)
                {
                    unsigned int index = parse_face_vertex(vertex_str, temp_pos, temp_tex, temp_norm, known_vertices, model);
                    face_indices.push_back(index);
                }

                if (face_indices.size() >= 3)
                {
                    for (size_t i = 1; i + 1 < face_indices.size(); ++i)
                    {
                        model.indices.push_back(face_indices[0]);
                        model.indices.push_back(face_indices[i]);
                        model.indices.push_back(face_indices[i + 1]);
                    }
                }
            }
        }

        return model;
    }

    std::vector<std::uint8_t> load_bytes_texture(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file)
            throw std::runtime_error("Failed to open texture file!");

        // Read all bytes into a vector
        std::vector<std::uint8_t> bytes(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        return bytes;
    }
}