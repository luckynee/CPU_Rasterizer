#include <iostream>
#include <SDL2/SDL.h>
#include <helper/renderer.hpp>

int main()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return -1;

    // Create window
    SDL_Window *window = SDL_CreateWindow("CPU Rasterizer",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);

    if (window == nullptr)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Main loop flag
    bool quit = false;
    SDL_Event e;

    SDL_Surface *draw_surface = nullptr;

    // Main loop
    while (!quit)
    {
        // Handle events
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
                if (draw_surface)
                    SDL_FreeSurface(draw_surface);
                draw_surface = nullptr;
            }

            if (!draw_surface)
            {
                draw_surface = SDL_CreateRGBSurfaceWithFormat(0, 800, 600, 32, SDL_PIXELFORMAT_RGBA32);
                SDL_SetSurfaceBlendMode(draw_surface, SDL_BLENDMODE_NONE);
            }

            rasterizer::image_view color_buffer{
                .pixels = (rasterizer::color4ub *)draw_surface->pixels,
                .width = 800,
                .height = 600,
            };

            rasterizer::clear(color_buffer, {0, 0, 255, 0});

            SDL_Rect rect{.x = 0, .y = 0, .w = 800, .h = 600};
            SDL_BlitSurface(draw_surface, nullptr, SDL_GetWindowSurface(window), &rect);

            SDL_UpdateWindowSurface(window);
        }
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}