#pragma once

#include <limits>
#include <memory>
#include <vector>

#include "application/application.hpp"
#include "types.hpp"
#include "model.hpp"
#include "types_math.hpp"

namespace rasterizer
{
    class rasterizer_engine
    {
    public:
        int m_width;
        int m_height;
        vector2f m_screen;
        color4ub m_clear_color = {0, 0, 0, 255};

        rasterizer_engine(int width, int height, application::application &app)
            : m_width(width),
              m_height(height),
              m_screen(static_cast<float>(width), static_cast<float>(height)),
              m_app(&app)
        {
            m_camera.camera_transform.position = {0, 0, -5.0f};
            m_color_buffer = static_cast<std::uint32_t *>(m_app->m_draw_surface->pixels);
            m_depth_buffer.resize(m_width * m_height, std::numeric_limits<float>::infinity());
        }

        virtual void setup_models() = 0;

        void pre_renders();

        virtual void render_models() = 0;

        //
        // Camera Functions
        //

        void rotate_camera(int xrel, int yrel);

        void move_camera(const Uint8 *key_state);

    protected:
        application::application *m_app = nullptr;
        std::uint32_t *m_color_buffer = nullptr;
        std::vector<float> m_depth_buffer;

        rasterizer::camera m_camera;
        std::vector<rasterizer::model> m_models;
        std::vector<std::unique_ptr<rasterizer::shader>> m_shaders;

        void clear_buffers();

        void draw_to_pixel_tiled(const model &model,
                                 std::vector<float> &depth_buffer,
                                 std::uint32_t *pixels);

        bool is_model_visible(const model &m, const camera &cam);
    };

    //
    // Derived Classes
    //

    class main_engine : public rasterizer_engine
    {
    public:
        using rasterizer_engine::rasterizer_engine;

        void setup_models() override;

        void render_models() override;
    };

    //
    // Global Function
    //

    void center_model(helper::model_data &model_data);

    inline rasterizer::vector3f transform_point(const rasterizer::vector3f &point, const rasterizer::transform &t)
    {
        rasterizer::vector3f scaled{
            point.x * t.scale.x,
            point.y * t.scale.y,
            point.z * t.scale.z};

        rasterizer::vector3f world{
            scaled.x + t.position.x,
            scaled.y + t.position.y,
            scaled.z + t.position.z};

        return world;
    }

}
