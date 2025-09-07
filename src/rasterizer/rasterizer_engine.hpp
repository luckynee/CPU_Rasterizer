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

        void setup_models();

        void pre_renders();

        void render_models();

        //
        // Camera Functions
        //

        void rotate_camera(int xrel, int yrel);

        void move_camera(const Uint8 *key_state);

    private:
        application::application *m_app = nullptr;
        std::uint32_t *m_color_buffer = nullptr;
        std::vector<float> m_depth_buffer;

        rasterizer::camera m_camera;
        std::vector<rasterizer::model> m_models;
        std::vector<std::unique_ptr<rasterizer::shader>> m_shaders;

        void clear_buffers();

        void draw_to_pixel_multithreaded(const model &model,
                                         std::vector<float> &depth_buffer,
                                         std::uint32_t *pixels);

        void draw_to_pixel(
            const model &model,
            std::vector<float> &depth_buffer,
            std::uint32_t *pixels);
    };

    //
    // Global Function
    //

    void center_model(helper::model_data &model_data);

}
