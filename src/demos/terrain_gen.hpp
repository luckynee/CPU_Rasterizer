#pragma once

#include "core/rasterizer/types_math.hpp"
#include "core/shader/shader.hpp"
#include "core/helper/math.hpp"

namespace demo
{
    float calculate_elevation(rasterizer::vector2f position);

    rasterizer::vector2f calculate_jiggle(rasterizer::vector2f position);

    std::vector<rasterizer::vector3f> generate_point_map(int resolution, float world_size, rasterizer::vector2f grid_center);

    void add_triangle(const rasterizer::vector3f &a, const rasterizer::vector3f &b, const rasterizer::vector3f &c, rasterizer::mesh_data &out_vertex_data);

    rasterizer::mesh_data generate_terrain(int resolution, float world_size, rasterizer::vector2f grid_center);

    //
    // Shader
    //

    class terrain_shader : public rasterizer::shader
    {
    public:
        rasterizer::vector3f light_direction = {0.0f, 0.0f, 0.0f};
        std::vector<float> height = {0.0f, 0.6f, 2.5f, 12.0f};
        rasterizer::vector3f sky_color = {0.6f, 0.8f, 1.0f};
        std::vector<rasterizer::vector3f> colours = {
            {0.9216f, 0.8039f, 0.3686f}, // Sand
            {0.2f, 0.6f, 0.98f},         // Water
            {0.2f, 0.6f, 0.1f},          // Grass
            {0.5f, 0.35f, 0.3f},         // Mountain
            {0.93f, 0.93f, 0.91f}};      // Snow

        terrain_shader(const rasterizer::vector3f &light_dir) : light_direction(std::move(light_dir)) {}

        rasterizer::vector3f shade(const rasterizer::vector3f &position,
                                   const rasterizer::vector3f &normal,
                                   const rasterizer::vector2f &tex_coord) const override
        {
            float triangle_height = tex_coord.x;
            rasterizer::vector3f terrain_color = colours[0];

            for (unsigned int i = 0; i < height.size(); i++)
            {
                if (triangle_height > height[i])
                    terrain_color = colours[i + 1];
                else
                    break;
            }

            float light_intensity = (rasterizer::dot(rasterizer::normalized_vector(normal), light_direction) + 1.0f) * 0.5f;
            terrain_color = terrain_color * light_intensity;

            constexpr float atmosphere_density = 0.0075f;
            float aerial_perspective_t = 1.0f - math::exp(-position.z * atmosphere_density);
            rasterizer::vector3f final_color = math::lerp(terrain_color, sky_color, aerial_perspective_t);

            return final_color;
        }
    };

    class cloud_shader : public rasterizer::shader
    {
    public:
        rasterizer::vector3f light_direction = {0.0f, 0.0f, 0.0f};
        rasterizer::vector3f tint;
        rasterizer::vector3f atmos_col;

        cloud_shader(const rasterizer::vector3f &light_dir, const rasterizer::vector3f &tint_col)
            : light_direction(std::move(light_dir)), tint(tint_col) {}

        rasterizer::vector3f shade(const rasterizer::vector3f &position,
                                   const rasterizer::vector3f &normal,
                                   const rasterizer::vector2f &tex_coord) const override
        {
            rasterizer::vector3f norm = rasterizer::normalized_vector(normal);
            float light_intensity = (rasterizer::dot(norm, light_direction) + 1.0f) * 0.5f;
            light_intensity = math::lerp(0.8f, 1.0f, light_intensity);

            float t = 1 - math::exp(-position.z * 0.0075f);
            rasterizer::vector3f final_color = math::lerp(tint * light_intensity, atmos_col, t);
            return final_color;
        }
    };
};