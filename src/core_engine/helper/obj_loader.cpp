#include <sstream>
#include <fstream>

#include "helper/obj_loader.hpp"

namespace helper
{
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
        if (pos_index > 0 && pos_index <= temp_pos.size())
            out_model_data.mesh.positions.push_back(temp_pos[pos_index - 1]);
        else
            out_model_data.mesh.positions.push_back(rasterizer::vector3f{0, 0, 0}); // fallback

        // Add tex coord
        if (tex_index > 0 && tex_index <= temp_tex.size())
            out_model_data.mesh.tex_coords.push_back(temp_tex[tex_index - 1]);
        else
            out_model_data.mesh.tex_coords.push_back(rasterizer::vector2f{0, 0}); // fallback

        // Add normal
        if (norm_index > 0 && norm_index <= temp_norm.size())
            out_model_data.mesh.normals.push_back(temp_norm[norm_index - 1]);
        else
            out_model_data.mesh.normals.push_back(rasterizer::vector3f{0, 0, 1}); // fallback

        unsigned int new_index = out_model_data.mesh.positions.size() - 1;
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

    rasterizer::texture create_texture_from_bytes(const std::vector<std::uint8_t> &bytes)
    {
        if (bytes.size() < 4)
            throw std::runtime_error("Invalid texture bytes");

        int width = bytes[0] | (bytes[1] << 8);
        int height = bytes[2] | (bytes[3] << 8);

        if (width <= 0 || height <= 0)
            throw std::runtime_error("Invalid texture bytes");

        std::size_t expected_size = 4 + (static_cast<std::size_t>(width) * height * 3);
        if (bytes.size() < expected_size)
            throw std::runtime_error("Invalid texture bytes");

        std::vector<rasterizer::vector3f> image_data(width * height);
        std::size_t byte_index = 4;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                float b = bytes[byte_index + 0] / 255.0f;
                float r = bytes[byte_index + 1] / 255.0f;
                float g = bytes[byte_index + 2] / 255.0f;

                image_data[y * width + x] = rasterizer::vector3f{r, g, b};
                byte_index += 3;
            }
        }

        return rasterizer::texture(width, height, std::move(image_data));
    }
}