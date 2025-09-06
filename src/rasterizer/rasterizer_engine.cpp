#include "rasterizer_engine.hpp"

namespace rasterizer
{
    void rasterizer_engine::draw_to_pixel(
        const model &model,
        std::vector<float> &depth_buffer,
        std::uint32_t *pixels)
    {
        for (unsigned int i = 0; i < model.triangles_data.size(); ++i)
        {
            const auto &triangle = model.triangles_data[i];

            // Skip triangles that are completely outside the screen
            if (triangle.inv_depth.z <= 0 || triangle.inv_depth.y <= 0 || triangle.inv_depth.x <= 0)
                continue;

            if (triangle.maxX < 0 || triangle.minX > m_screen.x - 1 ||
                triangle.maxY < 0 || triangle.minY > m_screen.y - 1)
                continue; // Triangle is completely outside the screen

            // Rasterize triangle within its bounding box
            int x_start = math::clamp(static_cast<int>(math::floor(triangle.minX)), 0, static_cast<int>(m_screen.x) - 1);
            int x_end = math::clamp(static_cast<int>(math::ceil(triangle.maxX)), 0, static_cast<int>(m_screen.x) - 1);
            int y_start = math::clamp(static_cast<int>(math::floor(triangle.minY)), 0, static_cast<int>(m_screen.y) - 1);
            int y_end = math::clamp(static_cast<int>(math::ceil(triangle.maxY)), 0, static_cast<int>(m_screen.y) - 1);

            for (int y = y_start; y <= y_end; ++y)
            {
                for (int x = x_start; x <= x_end; ++x)
                {
                    float px = static_cast<float>(x) + 0.5f;
                    float py = static_cast<float>(y) + 0.5f;
                    rasterizer::vector3f weight{0.0f, 0.0f, 0.0f};

                    if (!rasterizer::point_in_triangle(triangle.p0, triangle.p1, triangle.p2, px, py, weight))
                        continue;

                    float interpolated_z = 1.0f / (triangle.inv_depth.x * weight.x +
                                                   triangle.inv_depth.y * weight.y +
                                                   triangle.inv_depth.z * weight.z);
                    int idx = y * m_screen.x + x;

                    if (interpolated_z < depth_buffer[idx])
                    {
                        // Interpolate tex coord
                        vector2f tex_coord = (triangle.tx * weight.x +
                                              triangle.ty * weight.y +
                                              triangle.tz * weight.z) *
                                             interpolated_z;

                        depth_buffer[idx] = interpolated_z;

                        // Interpolate normal
                        vector3f normal =
                            (triangle.nx * weight.x +
                             triangle.ny * weight.y +
                             triangle.nz * weight.z) *
                            interpolated_z;

                        if (model.shader_ptr)
                        {
                            pixels[idx] = rasterizer::to_uint32(model.shader_ptr->shade(
                                rasterizer::vector3f{0, 0, 0}, // position not used
                                normal,
                                tex_coord));
                        }
                        else
                        {
                            pixels[idx] = rasterizer::to_uint32(model.triangle_colors[i]);
                        }
                    }
                }
            }
        }
    }
}