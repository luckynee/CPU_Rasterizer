#pragma once

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

    struct model
    {
        std::vector<vector3f> vertices;
        std::vector<vector3f> triangleCols;
        std::vector<triangle_data> triangles;

        model(std::vector<vector3f> verts, std::vector<vector3f> triCols)
            : vertices(std::move(verts)), triangleCols(std::move(triCols)) {}

        void fill_triangle_data(const vector2f &screen, transform &modelTransform, float fov);

        void draw_to_pixel(const vector2f &screen, std::vector<float> &depth_buffer, std::uint32_t *pixels);
    };

    struct transform
    {
        float yaw = 0;
        float pitch = 0;
        vector3f position;

        vector3f to_world_position(vector3f p)
        {
            set_basic_vector();
            return transform_vector(get_ihat(), get_jhat(), get_khat(), p) + position;
        }

        void set_basic_vector()
        {
            ihat_yaw = {math::cos(yaw), 0, -math::sin(yaw)};
            jhat_yaw = {0, 1, 0};
            khat_yaw = {math::sin(yaw), 0, math::cos(yaw)};
            ihat_pitch = {1, 0, 0};
            jhat_pitch = {0, math::cos(pitch), -math::sin(pitch)};
            khat_pitch = {0, math::sin(pitch), math::cos(pitch)};
        }

        vector3f get_ihat()
        {
            vector3f ihat = transform_vector(ihat_yaw, jhat_yaw, khat_yaw, ihat_pitch);
            return ihat;
        }

        vector3f get_jhat()
        {
            vector3f jhat = transform_vector(ihat_yaw, jhat_yaw, khat_yaw, jhat_pitch);
            return jhat;
        }

        vector3f get_khat()
        {
            vector3f khat = transform_vector(ihat_yaw, jhat_yaw, khat_yaw, khat_pitch);
            return khat;
        }

        vector3f transform_vector(vector3f ihat, vector3f jhat, vector3f khat, vector3f vec)
        {
            return {
                vec.x * ihat.x + vec.y * jhat.x + vec.z * khat.x,
                vec.x * ihat.y + vec.y * jhat.y + vec.z * khat.y,
                vec.x * ihat.z + vec.y * jhat.z + vec.z * khat.z};
        }

    private:
        vector3f ihat_yaw;
        vector3f jhat_yaw;
        vector3f khat_yaw;
        vector3f ihat_pitch;
        vector3f jhat_pitch;
        vector3f khat_pitch;
    };

    // TODO -> Fix to NDC later
    // TODO -> Maybe move this to camera class
    vector3f vertex_to_screen(const vector3f &vertex, transform &transform, const vector2f &screen, float fov);

    // TODO -> Move this later
    float calculate_dolly_zoom_fov(float fovInitial, float zPosInitial, float zPosCurrent);
}