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

    rasterizer::vector3f centroid{0, 0, 0};
    for (const auto &v : modelPoints)
    {
        centroid.x += v.x;
        centroid.y += v.y;
        centroid.z += v.z;
    }
    centroid.x /= modelPoints.size();
    centroid.y /= modelPoints.size();
    centroid.z /= modelPoints.size();

    for (auto &v : modelPoints)
    {
        v = v - centroid;
    }

    std::vector<rasterizer::model> models;
    unsigned int total_vertices = 0;

    for (unsigned int i = 0; i < 3; ++i)
    {
        rasterizer::transform modelTransform;

        if (i % 2 == 0)
            modelTransform.position = {static_cast<float>(i) * -1.0f, 1.0f, 3.0f + static_cast<float>(i)};
        else
            modelTransform.position = {static_cast<float>(i) * 1.0f, 1.0f, 3.0f + static_cast<float>(i)};
        models.emplace_back(modelPoints, triangleColors, modelTransform);
        total_vertices += modelPoints.size();
    }

    for (unsigned int i = 0; i < 3; ++i)
    {
        rasterizer::transform modelTransform;

        if (i % 2 == 0)
            modelTransform.position = {static_cast<float>(i) * -1.0f, -1.0f, 3.0f + static_cast<float>(i)};
        else
            modelTransform.position = {static_cast<float>(i) * 1.0f, -1.0f, 3.0f + static_cast<float>(i)};
        models.emplace_back(modelPoints, triangleColors, modelTransform);
        total_vertices += modelPoints.size();
    }

    // model transform

    // Center model

    // Init Data
    float fov = 90.0f;
    // float original_z = myModel.model_transform.position.z;
    // float original_fov = fov;

    // Screen
    rasterizer::vector2f screen{width, height};
    std::uint32_t *pixels = nullptr;

    // TODO -> maybe move this somwhere
    // Depth Buffer
    std::vector<float> depth_buffer(width * height, std::numeric_limits<float>::infinity());

    // Camera
    rasterizer::camera cam;
    cam.camera_transform.position = {0, 0, -5.0f};
    constexpr float cam_speed = 5.5f;
    constexpr float mouse_sensitivity = 2.0f;
    rasterizer::vector3f move_delta{0.0f, 0.0f, 0.0f};
    rasterizer::vector3f cam_forward;
    rasterizer::vector3f cam_right;

    // FPS
    std::uint32_t last_fps_time = SDL_GetTicks();
    int frame_count = 0;
    float fps = 0.0f;

    // Delta time
    std::uint32_t last_delta_time = SDL_GetTicks();
    float delta_time = 0.0f;

    // Main loop
    while (!quit)
    {
        std::uint32_t current_delta_time = SDL_GetTicks();
        delta_time = (current_delta_time - last_delta_time) / 1000.0f; // in seconds
        last_delta_time = current_delta_time;

        // TODO -> fix this later ( move to better place)
        move_delta.reset_to_zero();

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

            // Mouse movement Input
            if (e.type == SDL_MOUSEMOTION && e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                rasterizer::vector2f mouse_delta = rasterizer::vector2f{static_cast<float>(e.motion.xrel), static_cast<float>(e.motion.yrel)} / width * mouse_sensitivity;
                cam.camera_transform.pitch = math::clamp(cam.camera_transform.pitch + mouse_delta.y, -math::to_radians(89.0f), math::to_radians(89.0f));
                cam.camera_transform.yaw += mouse_delta.x;
            }

            // Keyboard Input
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_a)
                    move_delta -= cam_right;
                if (e.key.keysym.sym == SDLK_d)
                    move_delta += cam_right;
                if (e.key.keysym.sym == SDLK_w)
                    move_delta += cam_forward;
                if (e.key.keysym.sym == SDLK_s)
                    move_delta -= cam_forward;
            }

            // if (e.type == SDL_MOUSEWHEEL)
            // {
            //     if (e.wheel.y > 0) // scroll up
            //     {
            //         modelTransform.position.z -= 0.1f;
            //         fov = rasterizer::calculate_dolly_zoom_fov(original_fov, original_z, modelTransform.position.z);
            //     }
            //     else if (e.wheel.y < 0) // scroll down
            //     {
            //         modelTransform.position.z += 0.1f;
            //         fov = rasterizer::calculate_dolly_zoom_fov(original_fov, original_z, modelTransform.position.z);
            //     }
            // }
        }

        if (!draw_surface)
        {
            draw_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
            SDL_SetSurfaceBlendMode(draw_surface, SDL_BLENDMODE_NONE);
            pixels = static_cast<std::uint32_t *>(draw_surface->pixels);
        }

        // Clear Color Buffer
        std::fill(pixels, pixels + (width * height), 0x00000000);

        // Clear Depth Buffer
        std::fill(depth_buffer.begin(), depth_buffer.end(), std::numeric_limits<float>::infinity());

        // TODO -> maybe implement this in its ow class
        // Camera
        auto [right, up, forward] = cam.camera_transform.get_basis_vector();
        cam_forward = forward;
        cam_right = right;
        cam.camera_transform.position += rasterizer::normalized_vector(move_delta) * cam_speed * delta_time;
        cam.camera_transform.position.y = 1;

        // Draw Model
        for (auto &m : models)
        {
            m.model_transform.yaw += 0.005f;
            m.model_transform.pitch += 0.003f;
            m.fill_triangle_data(screen, cam);
            m.draw_to_pixel(screen, depth_buffer, pixels);
        }

        // Show FPS
        frame_count++;
        std::uint32_t current_time = SDL_GetTicks();
        if (current_time - last_fps_time >= 1000)
        {
            fps = frame_count * 1000.0f / (current_time - last_fps_time);
            std::cout << "FPS: " << fps << " FOV: " << fov << " total vertices: " << total_vertices << "\n";

            last_fps_time = current_time;

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
