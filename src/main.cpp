#include <iostream>
#include <SDL2/SDL.h>

#include <helper/obj_loader.hpp>

#include <rasterizer/math.hpp>
#include <rasterizer/model.hpp>
#include <rasterizer/types.hpp>

#include <vector>

// TODO -> change this later, only placeholder
constexpr int width = 800;
constexpr int height = 600;

int main()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return -1;

    // Create window
    SDL_Window *window = SDL_CreateWindow("CPU Rasterizer",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          width, height,
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
    float fov = 90.0f;
    std::string objPath = "../resource/model/monkey.obj";
    std::vector<rasterizer::vector3f> modelPoints = helper::load_obj(objPath);

    // Create Model
    std::vector<rasterizer::vector3f> triangleColors;
    rasterizer::Random rng(10);
    for (size_t i = 0; i < modelPoints.size(); i += 3)
    {
        triangleColors.push_back(rng.next_vector3f(0.0f, 1.0f));
    }
    rasterizer::model myModel(std::move(modelPoints), std::move(triangleColors));

    // model transform
    rasterizer::transform modelTransform;
    modelTransform.position = {0, 0, 1.5f};

    // Center model
    rasterizer::vector3f centroid{0, 0, 0};
    for (const auto &v : myModel.vertices)
    {
        centroid.x += v.x;
        centroid.y += v.y;
        centroid.z += v.z;
    }
    centroid.x /= myModel.vertices.size();
    centroid.y /= myModel.vertices.size();
    centroid.z /= myModel.vertices.size();

    for (auto &v : myModel.vertices)
    {
        v = v - centroid;
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

            if (e.type == SDL_MOUSEWHEEL)
            {
                if (e.wheel.y > 0) // scroll up
                {
                    float last_z = modelTransform.position.z;
                    modelTransform.position.z -= 0.1f;
                    fov = rasterizer::calculate_dolly_zoom_fov(fov, last_z, modelTransform.position.z);
                }
                else if (e.wheel.y < 0) // scroll down
                {
                    float last_z = modelTransform.position.z;
                    modelTransform.position.z += 0.1f;
                    fov = rasterizer::calculate_dolly_zoom_fov(fov, last_z, modelTransform.position.z);
                }
            }
        }

        if (!draw_surface)
        {
            draw_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
            SDL_SetSurfaceBlendMode(draw_surface, SDL_BLENDMODE_NONE);
        }

        // Clear Screen
        std::uint32_t *pixels = static_cast<std::uint32_t *>(draw_surface->pixels);
        std::fill(pixels, pixels + (width * height), 0x00000000);

        // Transform Model
        modelTransform.yaw += 0.005f;
        modelTransform.pitch += 0.003f;
        std::cout << "Z = " << modelTransform.position.z << " Fov = " << fov << std::endl;

        // Draw Model
        for (unsigned int i = 0; i < myModel.vertices.size(); i += 3)
        {
            rasterizer::vector2f a = rasterizer::vertex_to_screen(myModel.vertices[i + 0], modelTransform, rasterizer::vector2f{width, height}, fov);
            rasterizer::vector2f b = rasterizer::vertex_to_screen(myModel.vertices[i + 1], modelTransform, rasterizer::vector2f{width, height}, fov);
            rasterizer::vector2f c = rasterizer::vertex_to_screen(myModel.vertices[i + 2], modelTransform, rasterizer::vector2f{width, height}, fov);

            // Triangle bounds
            float minX = rasterizer::min(rasterizer::min(a.x, b.x), c.x);
            float minY = rasterizer::min(rasterizer::min(a.y, b.y), c.y);
            float maxX = rasterizer::max(rasterizer::max(a.x, b.x), c.x);
            float maxY = rasterizer::max(rasterizer::max(a.y, b.y), c.y);
            // Clamp to screen bounds
            minX = rasterizer::max(0.0f, rasterizer::min(static_cast<float>(width - 1), minX));
            maxX = rasterizer::max(0.0f, rasterizer::min(static_cast<float>(width - 1), maxX));
            minY = rasterizer::max(0.0f, rasterizer::min(static_cast<float>(height - 1), minY));
            maxY = rasterizer::max(0.0f, rasterizer::min(static_cast<float>(height - 1), maxY));

            for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y)
            {
                for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); ++x)
                {
                    rasterizer::vector2f p{static_cast<float>(x), static_cast<float>(y)};
                    if (rasterizer::point_in_triangle(a, b, c, p))
                    {
                        rasterizer::color4ub col = rasterizer::to_color4ub(myModel.triangleCols[i / 3]);
                        pixels[y * width + x] = rasterizer::to_uint32(col);
                    }
                }
            }
        }

        // Copy Data from surface
        SDL_Rect rect{.x = 0, .y = 0, .w = width, .h = height};
        SDL_BlitSurface(draw_surface, nullptr, SDL_GetWindowSurface(window), &rect);

        SDL_UpdateWindowSurface(window);
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
