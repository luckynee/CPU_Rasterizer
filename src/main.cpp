#include <iostream>
#include <SDL2/SDL.h>

#include <rasterizer/types.hpp>
#include <rasterizer/math.hpp>

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

    // Init Data
    constexpr unsigned int triangleCount = 250;
    rasterizer::vector2f positions[triangleCount * 3];
    rasterizer::vector2f velocities[triangleCount * 3];
    rasterizer::vector3f triangleCols[triangleCount];

    rasterizer::vector2f boundingBox{800.0f, 600.0f};
    rasterizer::vector2f triangleSize = boundingBox / 5.0f;
    rasterizer::Random randomGen(42);

    // For triangle positions
    for (unsigned int i = 0; i < triangleCount * 3; ++i)
    {
        positions[i] = triangleSize + (randomGen.next_vector2f(boundingBox.x, boundingBox.y) - triangleSize) * 0.3f;
    }

    // Init Velocity and color
    for (unsigned int i = 0; i < triangleCount * 3; i += 3)
    {

        float angle = randomGen.next_float(0.0f, 2.0f * 3.14159f);
        float speed = randomGen.next_float(50.0f, 150.0f);

        rasterizer::vector2f velocity = rasterizer::vector2f{std::cos(angle), std::sin(angle)} * speed;

        velocities[i + 0] = velocity;
        velocities[i + 1] = velocity;
        velocities[i + 2] = velocity;
        triangleCols[i / 3] = randomGen.next_vector3f();
    }
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
        }

        if (!draw_surface)
        {
            draw_surface = SDL_CreateRGBSurfaceWithFormat(0, 800, 600, 32, SDL_PIXELFORMAT_RGBA32);
            SDL_SetSurfaceBlendMode(draw_surface, SDL_BLENDMODE_NONE);
        }

        // update triangle positions
        for (unsigned int i = 0; i < triangleCount * 3; ++i)
        {
            positions[i] += velocities[i] * 0.016f; // Assuming ~60 FPS, so ~16ms per frame

            // Bounce off walls
            if (positions[i].x < 0.0f || positions[i].x > boundingBox.x)
                velocities[i].x = -velocities[i].x;
            if (positions[i].y < 0.0f || positions[i].y > boundingBox.y)
                velocities[i].y = -velocities[i].y;
        }

        // Clear Screen
        std::uint32_t *pixels = static_cast<std::uint32_t *>(draw_surface->pixels);
        std::fill(pixels, pixels + (800 * 600), 0x00000000);

        // Draw Triangle
        // ...existing code...
        for (unsigned int i = 0; i < triangleCount; ++i)
        {
            rasterizer::vector2f a = positions[i * 3 + 0];
            rasterizer::vector2f b = positions[i * 3 + 1];
            rasterizer::vector2f c = positions[i * 3 + 2];

            rasterizer::vector3f col = triangleCols[i];
            std::uint32_t color = rasterizer::to_uint32(rasterizer::to_color4ub(col));

            // Compute bounding box for this triangle
            float minX = rasterizer::min(rasterizer::min(a.x, b.x), c.x);
            float minY = rasterizer::min(rasterizer::min(a.y, b.y), c.y);
            float maxX = rasterizer::max(rasterizer::max(a.x, b.x), c.x);
            float maxY = rasterizer::max(rasterizer::max(a.y, b.y), c.y);

            // Clamp to screen bounds
            minX = rasterizer::max(0.0f, rasterizer::min(799.0f, minX));
            maxX = rasterizer::max(0.0f, rasterizer::min(799.0f, maxX));
            minY = rasterizer::max(0.0f, rasterizer::min(599.0f, minY));
            maxY = rasterizer::max(0.0f, rasterizer::min(599.0f, maxY));

            for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y)
            {
                for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); ++x)
                {
                    rasterizer::vector2f p{static_cast<float>(x), static_cast<float>(y)};
                    if (rasterizer::point_in_triangle(a, b, c, p))
                    {
                        pixels[y * 800 + x] = color;
                    }
                }
            }
        }

        // Copy Data from surface
        SDL_Rect rect{.x = 0, .y = 0, .w = 800, .h = 600};
        SDL_BlitSurface(draw_surface, nullptr, SDL_GetWindowSurface(window), &rect);

        SDL_UpdateWindowSurface(window);
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
