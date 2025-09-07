#include "terrain_gen.hpp"
#include "vendor/OpenSimplexNoise.hpp"

#include "helper/math.hpp"
#include "rasterizer/types_math.hpp"

namespace demo
{
    constexpr int seed = 69;
    OpenSimplexNoise::Noise noise_generator(seed);

    float calculate_elevation(rasterizer::vector2f position)
    {
        constexpr int layer_count = 5;
        constexpr int ridges_layer_start = 4;
        constexpr float lacunarity = 2.0f;
        constexpr float persistence = 0.5f;

        float frequency_initial = 0.035f;
        float amplitude_initial = 1.0f;
        float elevation = 0.0f;

        for (int i = 0; i < layer_count; i++)
        {
            float noise = noise_generator.eval(position.x * frequency_initial, position.y * frequency_initial);
            if (i >= ridges_layer_start)
                noise = 1.0 - std::abs(noise);

            elevation += noise * amplitude_initial;
            amplitude_initial *= persistence;
            frequency_initial *= lacunarity;
        }

        float v = math::lerp(1.0f, 0.8f, elevation);
        return elevation * math::pow(math::abs(elevation), 0.5f) * 18 * v;
    }

    rasterizer::vector2f calculate_jiggle(rasterizer::vector2f position)
    {
        const float jiggle = 0.7f;
        float ox = noise_generator.eval(position.x, position.y) * jiggle;
        float oy = noise_generator.eval(position.x - 10000, position.y - 10000) * jiggle;
        return rasterizer::vector2f{ox, oy};
    }

    std::vector<rasterizer::vector3f> generate_point_map(int resolution, float world_size, rasterizer::vector2f grid_center)
    {
        std::vector<rasterizer::vector3f> point_map = std::vector<rasterizer::vector3f>(resolution * resolution);

        for (int y = 0; y < resolution; y++)
        {
            for (int x = 0; x < resolution; x++)
            {
                rasterizer::vector2f local_grid_pos{static_cast<float>(x), static_cast<float>(y)};
                local_grid_pos = (local_grid_pos / static_cast<float>(resolution - 1) - rasterizer::vector2f{-1, 1});
                rasterizer::vector2f world_grid_pos;
                world_grid_pos.x = grid_center.x + world_size * local_grid_pos.x;
                world_grid_pos.y = grid_center.y + world_size * local_grid_pos.y;
                world_grid_pos += calculate_jiggle(world_grid_pos);
                float elevation = math::max(0.0f, calculate_elevation(world_grid_pos) + 0.8f);
                point_map[x + y * resolution] = rasterizer::vector3f{world_grid_pos.x, elevation, world_grid_pos.y};
            }
        }

        return point_map;
    }

    void add_triangle(const rasterizer::vector3f &a, const rasterizer::vector3f &b, const rasterizer::vector3f &c, rasterizer::mesh_data &out_vertex_data)
    {
        rasterizer::vector2f tex_coord{a.y + b.y + c.y, 0.0f};
        tex_coord = tex_coord / 3.0f;

        rasterizer::vector3f normal = rasterizer::cross(b - a, c - a);
        normal = rasterizer::normalized_vector(normal);

        out_vertex_data.positions.push_back(a);
        out_vertex_data.positions.push_back(b);
        out_vertex_data.positions.push_back(c);

        out_vertex_data.tex_coords.push_back(tex_coord);
        out_vertex_data.tex_coords.push_back(tex_coord);
        out_vertex_data.tex_coords.push_back(tex_coord);

        out_vertex_data.normals.push_back(normal);
        out_vertex_data.normals.push_back(normal);
        out_vertex_data.normals.push_back(normal);
    }

    rasterizer::mesh_data generate_terrain(int resolution, float world_size, rasterizer::vector2f grid_center)
    {
        std::vector<rasterizer::vector3f> point_map = generate_point_map(resolution, world_size, grid_center);

        rasterizer::mesh_data out_vertex_data;

        for (int y = 0; y < resolution; y++)
        {
            for (int x = 0; x < resolution; x++)
            {
                add_triangle(
                    point_map[x + y * resolution],
                    point_map[math::min(x + 1, resolution - 1) + y * resolution],
                    point_map[x + math::min(y + 1, resolution - 1) * resolution],
                    out_vertex_data);

                add_triangle(
                    point_map[math::min(x + 1, resolution - 1) + y * resolution],
                    point_map[math::min(x + 1, resolution - 1) + math::min(y + 1, resolution - 1) * resolution],
                    point_map[x + math::min(y + 1, resolution - 1) * resolution],
                    out_vertex_data);
            }
        }

        return out_vertex_data;
    }

}