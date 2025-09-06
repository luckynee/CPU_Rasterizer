#include "rasterizer/model.hpp"

#include <iostream>

// TODO -> REFACTOR THE RASTERIZER SO THE ENGINE HANDLE THE DRAWING TO PIXEL
// TODO -> CREATE ENGINE CLASS THAT WILL HANDLE WINDOW CREATION, RENDERING, INPUT, ETC

namespace rasterizer
{
    // Model Function
    void model::fill_triangle_data()
    {
        triangles_data.clear();

        for (unsigned int i = 0; i < rasterizer_indices.size(); i += 3)
        {
            unsigned int index0 = rasterizer_indices[i];
            unsigned int index1 = rasterizer_indices[i + 1];
            unsigned int index2 = rasterizer_indices[i + 2];

            const rasterizer::rasterizer_data &vert0_data = rasterizer_points[index0];
            const rasterizer::rasterizer_data &vert1_data = rasterizer_points[index1];
            const rasterizer::rasterizer_data &vert2_data = rasterizer_points[index2];

            rasterizer::vector2f p0 = vert0_data.position;
            rasterizer::vector2f p1 = vert1_data.position;
            rasterizer::vector2f p2 = vert2_data.position;

            // Triangle bounds
            float minX = math::min(math::min(p0.x, p1.x), p2.x);
            float minY = math::min(math::min(p0.y, p1.y), p2.y);
            float maxX = math::max(math::max(p0.x, p1.x), p2.x);
            float maxY = math::max(math::max(p0.y, p1.y), p2.y);

            rasterizer::vector3f inv_depth{1.0f / vert0_data.depth,
                                           1.0f / vert1_data.depth,
                                           1.0f / vert2_data.depth};
            rasterizer::vector2f tx{vert0_data.tex_coord.x * inv_depth.x,
                                    vert0_data.tex_coord.y * inv_depth.x};
            rasterizer::vector2f ty{vert1_data.tex_coord.x * inv_depth.y,
                                    vert1_data.tex_coord.y * inv_depth.y};
            rasterizer::vector2f tz{vert2_data.tex_coord.x * inv_depth.z,
                                    vert2_data.tex_coord.y * inv_depth.z};
            rasterizer::vector3f nx{vert0_data.normal.x * inv_depth.x,
                                    vert0_data.normal.y * inv_depth.x,
                                    vert0_data.normal.z * inv_depth.x};
            rasterizer::vector3f ny{vert1_data.normal.x * inv_depth.y,
                                    vert1_data.normal.y * inv_depth.y,
                                    vert1_data.normal.z * inv_depth.y};
            rasterizer::vector3f nz{vert2_data.normal.x * inv_depth.z,
                                    vert2_data.normal.y * inv_depth.z,
                                    vert2_data.normal.z * inv_depth.z};

            float denom = (p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y);
            if (std::abs(denom) < 1e-5f)
                continue; // Skip degenerate triangles
                          // ...existing code...

            triangles_data.emplace_back(rasterizer::triangle_data{p0, p1, p2, minX, maxX, minY, maxY, index0, index1, index2, inv_depth, tx, ty, tz, nx, ny, nz});
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

    vector2f view_to_screen(const vector3f &view_point, const vector2f &screen, camera &cam)
    {
        float screen_height_world = math::tan(cam.fov / 2) * 2;
        float pixel_per_world_unit = screen.y / screen_height_world / view_point.z;

        vector2f pixel_offset = vector2f(view_point.x, view_point.y) * pixel_per_world_unit;
        vector2f vertex_screen = screen / 2 + pixel_offset;
        vertex_screen.y = screen.y - vertex_screen.y;

        return vertex_screen;
    }

    vector3f vertex_to_view(const vector3f &vertex, transform &transform, camera &cam)
    {
        vector3f vertex_world = transform.to_world_position(vertex);
        vector3f vertex_view = cam.camera_transform.to_local_position(vertex_world);
        return vertex_view;
    }

    float calculate_dolly_zoom_fov(float fovInitial, float zPosInitial, float zPosCurrent)
    {
        float fov_in_rad = fovInitial * math::PI / 180.0f;
        float desired_half_height = math::tan(fov_in_rad / 2) * zPosInitial / zPosCurrent;
        return math::atan(desired_half_height) * 2 * 180.0f / math::PI;
    }

    // ...existing code...
    void process_model(rasterizer::model &m, camera &cam, vector2f &screen)
    {
        vector3f view_points[3];
        m.rasterizer_points.clear();
        m.rasterizer_indices.clear();

        for (unsigned int i = 0; i < m.indices.size(); i += 3)
        {
            view_points[0] = vertex_to_view(m.vertices[m.indices[i + 0]].position, m.model_transform, cam);
            view_points[1] = vertex_to_view(m.vertices[m.indices[i + 1]].position, m.model_transform, cam);
            view_points[2] = vertex_to_view(m.vertices[m.indices[i + 2]].position, m.model_transform, cam);

            constexpr float near_clip = 0.01f;
            bool clip0 = view_points[0].z <= near_clip;
            bool clip1 = view_points[1].z <= near_clip;
            bool clip2 = view_points[2].z <= near_clip;
            int clip_count = static_cast<int>(clip0) + static_cast<int>(clip1) + static_cast<int>(clip2);

            if (clip_count == 0)
            {
                add_vertex_to_rasterizer_points(m, view_points[0], m.indices[i + 0], screen, cam);
                add_vertex_to_rasterizer_points(m, view_points[1], m.indices[i + 1], screen, cam);
                add_vertex_to_rasterizer_points(m, view_points[2], m.indices[i + 2], screen, cam);
            }
            else if (clip_count == 1)
            {
                // Find clipped vertex
                int clip_index = clip0 ? 0 : (clip1 ? 1 : 2);
                int keep_a = (clip_index + 1) % 3;
                int keep_b = (clip_index + 2) % 3;

                vector3f clipped = view_points[clip_index];
                vector3f a = view_points[keep_a];
                vector3f b = view_points[keep_b];

                // Intersect clipped->a
                float t_a = (near_clip - clipped.z) / (a.z - clipped.z);
                vector3f intersect_a = math::lerp(clipped, a, t_a);

                // Intersect clipped->b
                float t_b = (near_clip - clipped.z) / (b.z - clipped.z);
                vector3f intersect_b = math::lerp(clipped, b, t_b);

                int vert_clip = m.indices[i + clip_index];
                int vert_a = m.indices[i + keep_a];
                int vert_b = m.indices[i + keep_b];

                // Only one triangle: [intersect_a, a, b], [intersect_a, b, intersect_b]
                add_vertex_to_rasterizer_points(m, intersect_a, vert_clip, vert_a, t_a, screen, cam);
                add_vertex_to_rasterizer_points(m, a, vert_a, screen, cam);
                add_vertex_to_rasterizer_points(m, b, vert_b, screen, cam);

                add_vertex_to_rasterizer_points(m, intersect_a, vert_clip, vert_a, t_a, screen, cam);
                add_vertex_to_rasterizer_points(m, b, vert_b, screen, cam);
                add_vertex_to_rasterizer_points(m, intersect_b, vert_clip, vert_b, t_b, screen, cam);
            }
            else if (clip_count == 2)
            {
                // Find non-clipped vertex
                int keep_index = (!clip0) ? 0 : (!clip1) ? 1
                                                         : 2;
                int clip_a = (keep_index + 1) % 3;
                int clip_b = (keep_index + 2) % 3;

                vector3f keep = view_points[keep_index];
                vector3f clipped_a = view_points[clip_a];
                vector3f clipped_b = view_points[clip_b];

                float t_a = (near_clip - keep.z) / (clipped_a.z - keep.z);
                float t_b = (near_clip - keep.z) / (clipped_b.z - keep.z);

                vector3f intersect_a = math::lerp(keep, clipped_a, t_a);
                vector3f intersect_b = math::lerp(keep, clipped_b, t_b);

                int vert_keep = m.indices[i + keep_index];
                int vert_a = m.indices[i + clip_a];
                int vert_b = m.indices[i + clip_b];

                // Only one triangle: [keep, intersect_a, intersect_b]
                add_vertex_to_rasterizer_points(m, keep, vert_keep, screen, cam);
                add_vertex_to_rasterizer_points(m, intersect_a, vert_keep, vert_a, t_a, screen, cam);
                add_vertex_to_rasterizer_points(m, intersect_b, vert_keep, vert_b, t_b, screen, cam);
            }
            // If all clipped, skip
        }
    }

    void add_vertex_to_rasterizer_points(rasterizer::model &m, vector3f view_point, int vert_index, vector2f &screen, camera &cam)
    {
        vector2f screen_pos = view_to_screen(view_point, screen, cam);
        vector2f tex_coord = m.vertices[vert_index].tex_coord;
        vector3f normal = m.vertices[vert_index].normal;
        float depth = view_point.z;
        m.rasterizer_points.emplace_back(rasterizer::rasterizer_data{screen_pos, tex_coord, normal, depth});
        m.rasterizer_indices.push_back(m.rasterizer_points.size() - 1);
    }

    void add_vertex_to_rasterizer_points(rasterizer::model &m, vector3f view_point, int vert_index_a, int vert_index_b, float t, vector2f &screen, camera &cam)
    {
        vector2f screen_pos = view_to_screen(view_point, screen, cam);
        vector2f tex_coord = math::lerp(m.vertices[vert_index_a].tex_coord, m.vertices[vert_index_b].tex_coord, t);
        vector3f normal = math::lerp(m.vertices[vert_index_a].normal, m.vertices[vert_index_b].normal, t);
        float depth = view_point.z;
        m.rasterizer_points.emplace_back(rasterizer::rasterizer_data{screen_pos, tex_coord, normal, depth});
        m.rasterizer_indices.push_back(m.rasterizer_points.size() - 1);
    }

}
