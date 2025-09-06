#include "application.hpp"

namespace application
{
    bool application::init_application()
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

        setup_world();

        return true;
    }

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

    void application::setup_world()
    {
    }

    void application::handle_events()
    {
        while (SDL_PollEvent(&m_event) != 0)
        {
            if (m_event.type == SDL_QUIT)
            {
                m_quit = true;
                if (m_draw_surface)
                    SDL_FreeSurface(m_draw_surface);
                m_draw_surface = nullptr;
            }
        }
    }

    void application::pre_render()
    {
    }

    void application::render()
    {
    }

    void application::post_render()
    {

        SDL_Rect rect{.x = 0, .y = 0, .w = m_width, .h = m_height};
        SDL_BlitSurface(m_draw_surface, nullptr, SDL_GetWindowSurface(m_window), &rect);

        SDL_UpdateWindowSurface(m_window);
    }
}