#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "helper/obj_loader.hpp"
#include "helper/math.hpp"
#include "rasterizer/model.hpp"
#include "rasterizer/types.hpp"

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

    // Create Model
    std::string objPath = "../resource/model/monkey.obj";
    std::vector<rasterizer::vector3f> modelPoints = helper::load_obj(objPath);

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

    // Init Data
    float fov = 90.0f;
    float original_z = modelTransform.position.z;
    float original_fov = fov;

    // Screen
    rasterizer::vector2f screen{width, height};

    // TODO -> maybe move this somwhere
    // Depth Buffer
    std::vector<float> depth_buffer(width * height, std::numeric_limits<float>::infinity());

    // FPS
    std::uint32_t last_time = SDL_GetTicks();
    int frame_count = 0;
    float fps = 0.0f;

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
                    modelTransform.position.z -= 0.1f;
                    fov = rasterizer::calculate_dolly_zoom_fov(original_fov, original_z, modelTransform.position.z);
                }
                else if (e.wheel.y < 0) // scroll down
                {
                    modelTransform.position.z += 0.1f;
                    fov = rasterizer::calculate_dolly_zoom_fov(original_fov, original_z, modelTransform.position.z);
                }
            }
        }

        if (!draw_surface)
        {
            draw_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
            SDL_SetSurfaceBlendMode(draw_surface, SDL_BLENDMODE_NONE);
        }

        // TODO -> maybe move this somwhere , and fix so not creating new buffer every frame
        // Clear Color Buffer
        std::uint32_t *pixels = static_cast<std::uint32_t *>(draw_surface->pixels);
        std::fill(pixels, pixels + (width * height), 0x00000000);

        // Clear Depth Buffer
        std::fill(depth_buffer.begin(), depth_buffer.end(), std::numeric_limits<float>::infinity());

        // Transform Model
        // modelTransform.yaw += 0.005f;
        modelTransform.pitch -= 0.005f;

        // Fill triangle data
        myModel.fill_triangle_data(screen, modelTransform, fov);

        // Draw each pixel
        myModel.draw_to_pixel(screen, depth_buffer, pixels);

        // // Find min/max depth (excluding infinity)
        // float min_depth = std::numeric_limits<float>::infinity();
        // float max_depth = 0.0f;
        // for (float d : depth_buffer)
        // {
        //     if (d < min_depth)
        //         min_depth = d;
        //     if (d > max_depth && d < std::numeric_limits<float>::infinity())
        //         max_depth = d;
        // }

        // // Write depth as grayscale to color buffer
        // for (int y = 0; y < height; ++y)
        // {
        //     for (int x = 0; x < width; ++x)
        //     {
        //         int idx = y * width + x;
        //         float d = depth_buffer[idx];
        //         uint8_t gray = 0;
        //         if (d < std::numeric_limits<float>::infinity())
        //         {// Find min/max depth (excluding infinity)
        // float min_depth = std::numeric_limits<float>::infinity();
        // float max_depth = 0.0f;
        // for (float d : depth_buffer)
        // {
        //     if (d < min_depth)
        //         min_depth = d;
        //     if (d > max_depth && d < std::numeric_limits<float>::infinity())
        //         max_depth = d;
        // }

        // // Write depth as grayscale to color buffer
        // for (int y = 0; y < height; ++y)
        // {
        //     for (int x = 0; x < width; ++x)
        //     {
        //         int idx = y * width + x;
        //         float d = depth_buffer[idx];
        //         uint8_t gray = 0;
        //         if (d < std::numeric_limits<float>::infinity())
        //         {
        //             float norm = (d - min_depth) / (max_depth - min_depth + 1e-6f); // avoid div by zero
        //             gray = static_cast<uint8_t>((1.0f - norm) * 255.0f);            // near=white, far=black
        //         }
        //         pixels[idx] = (gray << 16) | (gray << 8) | gray | (0xFF << 24); // RGBA
        //     }
        // }
        //             float norm = (d - min_depth) / (max_depth - min_depth + 1e-6f); // avoid div by zero
        //             gray = static_cast<uint8_t>((1.0f - norm) * 255.0f);            // near=white, far=black
        //         }
        //         pixels[idx] = (gray << 16) | (gray << 8) | gray | (0xFF << 24); // RGBA
        //     }
        // }

        // Show FPS
        frame_count++;
        std::uint32_t current_time = SDL_GetTicks();
        if (current_time - last_time >= 1000)
        {
            fps = frame_count * 1000.0f / (current_time - last_time);
            std::cout << "FPS: " << fps << " FOV: " << fov << " Pos : " << modelTransform.position.z << "\n";

            last_time = current_time;

            frame_count = 0;
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
