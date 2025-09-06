#pragma once

#include <SDL2/SDL.h>

#include "rasterizer/types.hpp"

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

        application(int width, int height) : m_width(width), m_height(height) {}

        bool init_application();

        void main_loop();

        void clean_up();

    private:
        void setup_world();

        void handle_events();

        void pre_render();

        void render();

        void post_render();
    };
}