#include <iostream>

#include "application.hpp"
#include "rasterizer/rasterizer_engine.hpp"

namespace application
{

    void application::main_loop()
    {
        while (!m_quit)
        {
            handle_events();

            pre_render();

            render();

            post_render();
        }
    }

    void application::clean_up()
    {
        if (m_draw_surface)
        {
            SDL_FreeSurface(m_draw_surface);
            m_draw_surface = nullptr;
        }

        if (m_window)
        {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }

        SDL_Quit();
    }

    //
    // Private methods
    //

    //
    // Application Specific Methods
    //

    void application::setup_world()
    {
        m_rasterizer_engine->setup_models();
    }

    void application::handle_events()
    {
        while (SDL_PollEvent(&m_event) != 0)
        {
            if (m_event.type == SDL_QUIT)
                m_quit = true;

            // Mouse
            if (m_event.type == SDL_MOUSEMOTION && m_event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))
                m_rasterizer_engine->rotate_camera(m_event.motion.xrel, m_event.motion.yrel);

            // Keyboard
            m_rasterizer_engine->move_camera(SDL_GetKeyboardState(NULL));
        }
    }

    void application::pre_render()
    {
        update_delta_time();
        update_fps();

        m_rasterizer_engine->pre_renders();
    }

    void application::render()
    {
        m_rasterizer_engine->render_models();
    }

    void application::post_render()
    {

        SDL_Rect rect{.x = 0, .y = 0, .w = m_width, .h = m_height};
        SDL_BlitSurface(m_draw_surface, nullptr, SDL_GetWindowSurface(m_window), &rect);

        SDL_UpdateWindowSurface(m_window);
    }

    //
    // Helper Methods
    //

    void application::update_fps()
    {
        frame_count++;
        std::uint32_t current_time = SDL_GetTicks();
        if (current_time - last_fps_time >= 1000)
        {
            fps = frame_count * 1000.0f / (current_time - last_fps_time);

            last_fps_time = current_time;

            std::cout << "FPS: " << fps << std::endl;

            frame_count = 0;
        }
    }

    void application::update_delta_time()
    {
        std::uint32_t current_time = SDL_GetTicks();
        delta_time = (current_time - last_delta_time) / 1000.0f;
        last_delta_time = current_time;
    }

    //
    // Derived Classes
    //

    bool main_application::init_application()
    {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
            return false;

        // Create window
        m_window = SDL_CreateWindow("CPU Rasterizer",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    m_width, m_height,
                                    SDL_WINDOW_SHOWN);

        if (m_window == nullptr)
        {
            SDL_Quit();
            return false;
        }

        m_event = SDL_Event();

        m_draw_surface = SDL_CreateRGBSurfaceWithFormat(0, m_width, m_height, 32, SDL_PIXELFORMAT_RGBA32);
        SDL_SetSurfaceBlendMode(m_draw_surface, SDL_BLENDMODE_NONE);

        m_rasterizer_engine = std::make_unique<rasterizer::main_engine>(m_width, m_height, *this);

        setup_world();

        return true;
    }
}