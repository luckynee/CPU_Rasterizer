#include "rasterizer/model.hpp"

namespace rasterizer
{
    // Model Function
    void model::fill_triangle_data(const vector2f &screen, float fov)
    {
        triangles_data.clear();

        for (unsigned int i = 0; i < vertices.size(); i += 3)
        {
            rasterizer::vector3f v0 = rasterizer::vertex_to_screen(vertices[i], model_transform, screen, fov);
            rasterizer::vector3f v1 = rasterizer::vertex_to_screen(vertices[i + 1], model_transform, screen, fov);
            rasterizer::vector3f v2 = rasterizer::vertex_to_screen(vertices[i + 2], model_transform, screen, fov);

            rasterizer::vector2f p0 = static_cast<rasterizer::vector2f>(v0);
            rasterizer::vector2f p1 = static_cast<rasterizer::vector2f>(v1);
            rasterizer::vector2f p2 = static_cast<rasterizer::vector2f>(v2);

            // Triangle bounds
            float minX = math::min(math::min(p0.x, p1.x), p2.x);
            float minY = math::min(math::min(p0.y, p1.y), p2.y);
            float maxX = math::max(math::max(p0.x, p1.x), p2.x);
            float maxY = math::max(math::max(p0.y, p1.y), p2.y);

            float denom = (p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y);

            triangles_data.emplace_back(rasterizer::triangle_data{v0, v1, v2, p0, p1, p2, minX, maxX, minY, maxY, denom});
        }
    }

    void model::draw_to_pixel(const vector2f &screen, std::vector<float> &depth_buffer, std::uint32_t *pixels)
    {
        for (unsigned int i = 0; i < triangles_data.size(); ++i)
        {
            const auto &triangle = triangles_data[i];

            // Rasterize triangle within its bounding box
            int x_start = math::clamp(static_cast<int>(math::floor(triangle.minX)), 0, static_cast<int>(screen.x) - 1);
            int x_end = math::clamp(static_cast<int>(math::ceil(triangle.maxX)), 0, static_cast<int>(screen.x) - 1);
            int y_start = math::clamp(static_cast<int>(math::floor(triangle.minY)), 0, static_cast<int>(screen.y) - 1);
            int y_end = math::clamp(static_cast<int>(math::ceil(triangle.maxY)), 0, static_cast<int>(screen.y) - 1);

            for (int y = y_start; y <= y_end; ++y)
            {
                for (int x = x_start; x <= x_end; ++x)
                {
                    float px = static_cast<float>(x) + 0.5f;
                    float py = static_cast<float>(y) + 0.5f;
                    rasterizer::vector3f weight{0.0f, 0.0f, 0.0f};

                    if (!rasterizer::point_in_triangle(triangle.v2a, triangle.v2b, triangle.v2c, px, py, weight, triangle.denom))
                        continue;

                    // Interpolate depth
                    float interpolated_z = weight.x * triangle.v3a.z + weight.y * triangle.v3b.z + weight.z * triangle.v3c.z;

                    int idx = y * screen.x + x;
                    if (interpolated_z < depth_buffer[idx])
                    {
                        depth_buffer[idx] = interpolated_z;
                        pixels[idx] = rasterizer::to_uint32(triangle_colors[i]);
                    }
                }
            }
        }
    }

    // Global function

    vector3f vertex_to_screen(const vector3f &vertex, transform &transform, const vector2f &screen, float fov)
    {
        vector3f world_pos = transform.to_world_position(vertex);

        // Convert FOV from degrees to radians
        float fov_rad = fov * math::PI / 180.0f;
        float screen_height_world = math::tan(fov_rad / 2) * 2;
        float pixel_per_world_unit = screen.y / screen_height_world / world_pos.z;

        vector2f pixel_offset = vector2f(world_pos.x, world_pos.y) * pixel_per_world_unit;
        vector2f vertex_screen = screen / 2 + pixel_offset;
        return {vertex_screen.x, vertex_screen.y, world_pos.z};
    }

    float calculate_dolly_zoom_fov(float fovInitial, float zPosInitial, float zPosCurrent)
    {
        float fov_in_rad = fovInitial * math::PI / 180.0f;
        float desired_half_height = math::tan(fov_in_rad / 2) * zPosInitial / zPosCurrent;
        return math::atan(desired_half_height) * 2 * 180.0f / math::PI;
    }
}