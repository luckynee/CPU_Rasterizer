#pragma once

#include <tuple>
#include <vector>

#include "rasterizer/types.hpp"

namespace rasterizer
{
    struct transform;

    struct triangle_data
    {
        vector3f v3a, v3b, v3c; // screen-space vertices
        vector2f v2a, v2b, v2c; // To avoid Conversion
        float minX, maxX, minY, maxY;
        float denom;
        // Add more as needed (color, normal, etc.)
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
            vector3f ihat_yaw = {math::cos(-yaw), 0, -math::sin(-yaw)};
            vector3f jhat_yaw = {0, 1, 0};
            vector3f khat_yaw = {math::sin(-yaw), 0, math::cos(-yaw)};
            vector3f ihat_pitch = {1, 0, 0};
            vector3f jhat_pitch = {0, math::cos(-pitch), -math::sin(-pitch)};
            vector3f khat_pitch = {0, math::sin(-pitch), math::cos(-pitch)};
            vector3f ihat = transform_vector(ihat_pitch, jhat_pitch, khat_pitch, ihat_yaw);
            vector3f jhat = transform_vector(ihat_pitch, jhat_pitch, khat_pitch, jhat_yaw);
            vector3f khat = transform_vector(ihat_pitch, jhat_pitch, khat_pitch, khat_yaw);

            return std::make_tuple(ihat, jhat, khat);
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
    };

    struct model
    {
        std::vector<vector3f> vertices;
        std::vector<vector3f> triangle_colors;
        transform model_transform;
        std::vector<triangle_data> triangles_data;

        model(std::vector<vector3f> verts, std::vector<vector3f> triCols, transform modelTransform)
            : vertices(std::move(verts)), triangle_colors(std::move(triCols)), model_transform(std::move(modelTransform)) {}

        void fill_triangle_data(const vector2f &screen, camera &cam);

        void draw_to_pixel(const vector2f &screen, std::vector<float> &depth_buffer, std::uint32_t *pixels);
    };

    // TODO -> Fix to NDC later
    // TODO -> Maybe move this to camera class
    vector3f vertex_to_screen(const vector3f &vertex, transform &transform, const vector2f &screen, camera &cam);

    // TODO -> Move this later
    float calculate_dolly_zoom_fov(float fovInitial, float zPosInitial, float zPosCurrent);

}