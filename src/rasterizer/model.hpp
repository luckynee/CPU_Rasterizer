#pragma once

#include <rasterizer/types.hpp>

#include <vector>

namespace rasterizer
{
    struct model
    {
        std::vector<vector3f> vertices;
        std::vector<vector3f> triangleCols;

        model(std::vector<vector3f> verts, std::vector<vector3f> triCols)
            : vertices(std::move(verts)), triangleCols(std::move(triCols)) {}
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
            ihat_yaw = {std::cos(yaw), 0, -std::sin(yaw)};
            jhat_yaw = {0, 1, 0};
            khat_yaw = {std::sin(yaw), 0, std::cos(yaw)};
            ihat_pitch = {1, 0, 0};
            jhat_pitch = {0, std::cos(pitch), -std::sin(pitch)};
            khat_pitch = {0, std::sin(pitch), std::cos(pitch)};
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
    inline vector2f vertex_to_screen(const vector3f &vertex, transform &transform, const vector2f &screen, float fov)
    {
        vector3f world_pos = transform.to_world_position(vertex);

        // Convert FOV from degrees to radians
        float fov_rad = fov * M_PI / 180.0f;
        float screen_height_world = std::tan(fov_rad / 2) * 2;
        float pixel_per_world_unit = screen.y / screen_height_world / world_pos.z;

        vector2f pixel_offset = vector2f(world_pos.x, world_pos.y) * pixel_per_world_unit;
        return screen / 2 + pixel_offset;
    }

    // TODO -> Move this later
    inline float calculate_dolly_zoom_fov(float fovInitial, float zPosInitial, float zPosCurrent)
    {
        float fov_in_rad = fovInitial * M_PI / 180.0f;
        float desired_half_height = std::tan(fov_in_rad / 2) * zPosInitial / zPosCurrent;
        return std::atan(desired_half_height) * 2 * 180.0f / M_PI;
    }

}