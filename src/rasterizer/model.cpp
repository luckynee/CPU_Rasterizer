#include "rasterizer/model.hpp"

namespace rasterizer
{
    // Model Function
    void model::fill_triangle_data(const vector2f &screen, camera &cam)
    {
        triangles_data.clear();

        for (unsigned int i = 0; i < indices.size(); i += 3)
        {
            unsigned int index0 = indices[i];
            unsigned int index1 = indices[i + 1];
            unsigned int index2 = indices[i + 2];

            const vertex_data &vert0_data = vertices[index0];
            const vertex_data &vert1_data = vertices[index1];
            const vertex_data &vert2_data = vertices[index2];

            rasterizer::vector3f v0 = rasterizer::vertex_to_screen(vert0_data.position, model_transform, screen, cam);
            rasterizer::vector3f v1 = rasterizer::vertex_to_screen(vert1_data.position, model_transform, screen, cam);
            rasterizer::vector3f v2 = rasterizer::vertex_to_screen(vert2_data.position, model_transform, screen, cam);

            rasterizer::vector2f p0 = static_cast<rasterizer::vector2f>(v0);
            rasterizer::vector2f p1 = static_cast<rasterizer::vector2f>(v1);
            rasterizer::vector2f p2 = static_cast<rasterizer::vector2f>(v2);

            // Triangle bounds
            float minX = math::min(math::min(p0.x, p1.x), p2.x);
            float minY = math::min(math::min(p0.y, p1.y), p2.y);
            float maxX = math::max(math::max(p0.x, p1.x), p2.x);
            float maxY = math::max(math::max(p0.y, p1.y), p2.y);

            float denom = (p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y);

            triangles_data.emplace_back(rasterizer::triangle_data{v0, v1, v2, p0, p1, p2, minX, maxX, minY, maxY, denom, index0, index1, index2});
        }
    }

    void model::draw_to_pixel(const vector2f &screen, std::vector<float> &depth_buffer, std::uint32_t *pixels)
    {
        for (unsigned int i = 0; i < triangles_data.size(); ++i)
        {
            const auto &triangle = triangles_data[i];

            // if triangle behind camera
            if (triangle.v3a.z <= 0 || triangle.v3b.z <= 0 || triangle.v3c.z <= 0)
                continue;

            if (triangle.maxX < 0 || triangle.minX > screen.x - 1 ||
                triangle.maxY < 0 || triangle.minY > screen.y - 1)
            {
                continue; // Triangle is completely outside the screen
            }

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
                    float denom = weight.x / triangle.v3a.z + weight.y / triangle.v3b.z + weight.z / triangle.v3c.z;

                    float interpolated_z = 1.0f / denom;

                    int idx = y * screen.x + x;
                    if (interpolated_z < depth_buffer[idx])
                    {
                        vector2f tex_coord =
                            vertices[triangle.idx0].tex_coord * (weight.x / triangle.v3a.z) +
                            vertices[triangle.idx1].tex_coord * (weight.y / triangle.v3b.z) +
                            vertices[triangle.idx2].tex_coord * (weight.z / triangle.v3c.z);
                        tex_coord = tex_coord * interpolated_z;

                        depth_buffer[idx] = interpolated_z;
                        // pixels[idx] = rasterizer::to_uint32(triangle_colors[i]);
                        if (shader_ptr)
                        {
                            pixels[idx] = rasterizer::to_uint32(shader_ptr->shade(
                                rasterizer::vector3f{0, 0, 0}, // position not used
                                rasterizer::vector3f{0, 0, 0}, // normal not used
                                tex_coord));
                        }
                        else
                        {
                            pixels[idx] = rasterizer::to_uint32(triangle_colors[i]);
                        }
                    }
                }
            }
        }
    }

    vector3f vertex_to_screen(const vector3f &vertex, transform &transform, const vector2f &screen, camera &cam)
    {
        vector3f vertex_world = transform.to_world_position(vertex);
        vector3f vertex_view = cam.camera_transform.to_local_position(vertex_world);

        float screen_height_world = math::tan(cam.fov / 2) * 2;
        float pixel_per_world_unit = screen.y / screen_height_world / vertex_view.z;

        vector2f pixel_offset = vector2f(vertex_view.x, vertex_view.y) * pixel_per_world_unit;
        vector2f vertex_screen = screen / 2 + pixel_offset;
        vertex_screen.y = screen.y - vertex_screen.y;

        return {vertex_screen.x, vertex_screen.y, vertex_view.z};
    }

    float calculate_dolly_zoom_fov(float fovInitial, float zPosInitial, float zPosCurrent)
    {
        float fov_in_rad = fovInitial * math::PI / 180.0f;
        float desired_half_height = math::tan(fov_in_rad / 2) * zPosInitial / zPosCurrent;
        return math::atan(desired_half_height) * 2 * 180.0f / math::PI;
    }
}