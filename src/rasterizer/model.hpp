#pragma once

#include <tuple>
#include <vector>

#include "rasterizer/types.hpp"
#include "shader/shader.hpp"

namespace rasterizer
{
    struct transform;

    struct triangle_data
    {
        vector3f v3a, v3b, v3c;
        vector2f v2a, v2b, v2c;
        float minX, maxX, minY, maxY, denom;
        unsigned int idx0, idx1, idx2;
    };

    struct transform
    {
        float yaw = 0;
        float pitch = 0;
        vector3f position{0, 0, 0};

        vector3f to_world_position(vector3f p)
        {
            auto [ihat, jhat, khat] = get_basis_vector();
            return transform_vector(ihat, jhat, khat, p) + position;
        }

        vector3f to_local_position(vector3f world_point)
        {
            auto [ihat, jhat, khat] = get_inverse_basis_vector();
            return transform_vector(ihat, jhat, khat, world_point - position);
        }

        std::tuple<vector3f, vector3f, vector3f> get_basis_vector()
        {
            vector3f ihat_yaw = {math::cos(yaw), 0, -math::sin(yaw)};
            vector3f jhat_yaw = {0, 1, 0};
            vector3f khat_yaw = {math::sin(yaw), 0, math::cos(yaw)};
            vector3f ihat_pitch = {1, 0, 0};
            vector3f jhat_pitch = {0, math::cos(pitch), -math::sin(pitch)};
            vector3f khat_pitch = {0, math::sin(pitch), math::cos(pitch)};

            vector3f ihat = transform_vector(ihat_yaw, jhat_yaw, khat_yaw, ihat_pitch);
            vector3f jhat = transform_vector(ihat_yaw, jhat_yaw, khat_yaw, jhat_pitch);
            vector3f khat = transform_vector(ihat_yaw, jhat_yaw, khat_yaw, khat_pitch);

            return std::make_tuple(ihat, jhat, khat);
        }

        std::tuple<vector3f, vector3f, vector3f> get_inverse_basis_vector()
        {
            auto [ihat, jhat, khat] = get_basis_vector();
            vector3f ihat_inverse{ihat.x, jhat.x, khat.x};
            vector3f jhat_inverse{ihat.y, jhat.y, khat.y};
            vector3f khat_inverse{ihat.z, jhat.z, khat.z};

            return std::make_tuple(ihat_inverse, jhat_inverse, khat_inverse);
        }

        vector3f transform_vector(vector3f ihat, vector3f jhat, vector3f khat, vector3f vec)
        {
            return {
                vec.x * ihat.x + vec.y * jhat.x + vec.z * khat.x,
                vec.x * ihat.y + vec.y * jhat.y + vec.z * khat.y,
                vec.x * ihat.z + vec.y * jhat.z + vec.z * khat.z};
        }
    };

    struct camera
    {
        float fov = math::to_radians(90.0f);
        transform camera_transform;
        vector3f cam_forward;
        vector3f cam_right;
        vector3f cam_up;

        void update_camera_vectors()
        {
            auto [right, up, forward] = camera_transform.get_basis_vector();
            cam_forward = forward;
            cam_right = right;
            cam_up = up;
        }

        void move_camera(const vector3f &delta, float cam_speed, float delta_time)
        {
            camera_transform.position += normalized_vector(delta) * cam_speed * delta_time;
            camera_transform.position.y = 1;
        }
    };

    struct model
    {
        std::vector<vertex_data> vertices;
        std::vector<unsigned int> indices;
        std::vector<vector3f> triangle_colors;
        transform model_transform;
        std::vector<triangle_data> triangles_data;
        const shader *shader_ptr;

        model(
            const std::vector<vertex_data> &verts,
            const std::vector<unsigned int> &inds,
            const std::vector<vector3f> &triCols,
            const transform &modelTransform,
            const shader *shaderPtr = nullptr)
            : vertices(std::move(verts)),
              indices(std::move(inds)),
              triangle_colors(std::move(triCols)),
              model_transform(std::move(modelTransform)),
              shader_ptr(shaderPtr) {}

        void fill_triangle_data(const vector2f &screen, camera &cam);

        void draw_to_pixel(const vector2f &screen, std::vector<float> &depth_buffer, std::uint32_t *pixels);
    };

    // TODO -> Fix to NDC later
    // TODO -> Maybe move this to camera class
    vector3f vertex_to_screen(const vector3f &vertex, transform &transform, const vector2f &screen, camera &cam);

    // TODO -> Move this later
    float calculate_dolly_zoom_fov(float fovInitial, float zPosInitial, float zPosCurrent);

}