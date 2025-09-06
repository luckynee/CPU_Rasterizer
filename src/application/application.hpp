#pragma once

#include <memory>

#include <SDL2/SDL.h>

#include "rasterizer/types.hpp"

namespace rasterizer
{
    class rasterizer_engine;
}

namespace application
{
    class application
    {
    public:
        int m_width;
        int m_height;

        bool m_quit = false;

        SDL_Window *m_window = nullptr;
        SDL_Surface *m_draw_surface = nullptr;
        SDL_Event m_event;

        std::unique_ptr<rasterizer::rasterizer_engine> m_rasterizer_engine;

        application(int width, int height) : m_width(width), m_height(height) {}

        bool init_application();

        void main_loop();

        void clean_up();

        //
        // Helper Methods
        //

        float get_fps() const { return fps; }

        float get_delta_time() const { return delta_time; }

    private:
        std::uint32_t last_fps_time = SDL_GetTicks();
        float fps = 0.0f;
        int frame_count = 0;

        std::uint32_t last_delta_time = SDL_GetTicks();
        float delta_time = 0.0f;

        //
        // Application Specific Methods
        //

        void setup_world();

        void handle_events();

        void pre_render();

        void render();

        void post_render();

        //
        // Helper Methods
        //

        void update_fps();

        void update_delta_time();
    };
}