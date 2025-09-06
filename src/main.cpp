#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "application/application.hpp"
#include "helper/obj_loader.hpp"
#include "helper/math.hpp"
#include "rasterizer/model.hpp"
#include "rasterizer/types.hpp"
#include "rasterizer/rasterizer_engine.hpp"
#include "shader/shader.hpp"

// TODO -> change this later, only placeholder
constexpr int width = 800;
constexpr int height = 600;

int main()
{
    application::application app(width, height);

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

    // // Create renderer
    // SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // if (renderer == nullptr)
    // {
    //     std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    //     SDL_DestroyWindow(window);
    //     SDL_Quit();
    //     return -1;
    // }

    // Main loop flag
    bool quit = false;
    SDL_Event e;

    SDL_Surface *draw_surface = nullptr;

    // Create Model

    // std::string objPath = "../resource/model/fox.obj";
    // helper::model_data loaded_model = helper::load_obj(objPath);
    // std::vector<rasterizer::vector3f> triangleColors;
    // rasterizer::Random rng(10);

    // for (size_t i = 0; i < loaded_model.indices.size(); i += 3)
    // {
    //     triangleColors.push_back(rng.next_vector3f(0.0f, 1.0f));
    // }

    // rasterizer::vector3f centroid{0.0f, 0.0f, 0.0f};
    // for (const auto &v : loaded_model.vertices)
    // {
    //     centroid.x += v.position.x;
    //     centroid.y += v.position.y;
    //     centroid.z += v.position.z;
    // }
    // centroid.x /= loaded_model.vertices.size();
    // centroid.y /= loaded_model.vertices.size();
    // centroid.z /= loaded_model.vertices.size();

    // for (auto &v : loaded_model.vertices)
    // {
    //     v.position = v.position - centroid;
    // }

    std::vector<rasterizer::model> models;
    int total_vertices = 0;

    // Texture
    // auto tex_bytes = helper::load_bytes_texture("../resource/textures/uvGrid.bytes");
    // rasterizer::texture my_texture = rasterizer::create_texture_from_bytes(tex_bytes);
    // rasterizer::texture_shader my_shader{my_texture};

    // Lit shader
    // rasterizer::lit_shader my_shader{rasterizer::vector3f{0.0f, 0.0f, -1.0f}};

    // Lit texture
    // auto tex_bytes = helper::load_bytes_texture("../resource/textures/colMap.bytes");
    // rasterizer::texture my_texture = helper::create_texture_from_bytes(tex_bytes);
    // rasterizer::lit_texture my_shader{my_texture, rasterizer::vector3f{0.0f, 0.0f, -1.0f}};

    // for (unsigned int i = 0; i < 1; ++i)
    // {
    //     rasterizer::transform model_transform;
    //     if (i % 2 == 0)
    //         model_transform.position = {static_cast<float>(i) * 1.5f, 0.0f, 0.0f};
    //     else
    //         model_transform.position = {static_cast<float>(i) * -1.5f, 0.0f, 0.0f};

    //     models.emplace_back(loaded_model.vertices, loaded_model.indices, triangleColors, model_transform, &my_shader);
    // }

    // floor
    std::string objPath2 = "../resource/model/floor.obj";
    helper::model_data loaded_model2 = helper::load_obj(objPath2);
    rasterizer::vector3f centroid2{0.0f, 0.0f, 0.0f};
    for (const auto &v : loaded_model2.vertices)
    {
        centroid2.x += v.position.x;
        centroid2.y += v.position.y;
        centroid2.z += v.position.z;
    }
    centroid2.x /= loaded_model2.vertices.size();
    centroid2.y /= loaded_model2.vertices.size();
    centroid2.z /= loaded_model2.vertices.size();

    for (auto &v : loaded_model2.vertices)
    {
        v.position = v.position - centroid2;
    }

    auto floor_bytes = helper::load_bytes_texture("../resource/textures/uvGrid.bytes");
    rasterizer::texture floor_texture = helper::create_texture_from_bytes(floor_bytes);
    rasterizer::texture_shader floor_shader{floor_texture};

    rasterizer::transform floor_transform;
    floor_transform.position = {0.0f, -2.0f, 0.0f};
    floor_transform.scale = {3.0f, 1.0f, 3.0f};

    models.emplace_back(loaded_model2.vertices, loaded_model2.indices, std::vector<rasterizer::vector3f>(loaded_model2.indices.size() / 3, rasterizer::vector3f{1.0f, 1.0f, 1.0f}), floor_transform, &floor_shader);

    // Init Data
    float fov = 90.0f;

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

    // FPS
    std::uint32_t last_fps_time = SDL_GetTicks();
    int frame_count = 0;
    float fps = 0.0f;

    // Delta time
    std::uint32_t last_delta_time = SDL_GetTicks();
    float delta_time = 0.0f;

    // Rasterizer engine
    rasterizer::rasterizer_engine engine(width, height);

    // Main loop
    while (!app.m_quit)
    {
        std::uint32_t current_delta_time = SDL_GetTicks();
        delta_time = (current_delta_time - last_delta_time) / 1000.0f; // in seconds
        last_delta_time = current_delta_time;

        // Handle events
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                app.m_quit = true;
                if (draw_surface)
                    SDL_FreeSurface(draw_surface);
                draw_surface = nullptr;
            }

            // Mouse movement Input
            if (e.type == SDL_MOUSEMOTION && e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                rasterizer::vector2f mouse_delta = rasterizer::vector2f{static_cast<float>(e.motion.xrel), static_cast<float>(e.motion.yrel)} / width * mouse_sensitivity;
                cam.camera_transform.pitch = math::clamp(cam.camera_transform.pitch - mouse_delta.y, -math::to_radians(89.0f), math::to_radians(89.0f));
                cam.camera_transform.yaw += mouse_delta.x;
            }

            // TODO -> fix this later ( move to better place)
            // Keyboard Input
            move_delta.reset_to_zero();
            const Uint8 *keystate = SDL_GetKeyboardState(NULL);
            if (keystate[SDL_SCANCODE_W])
                move_delta += cam.cam_forward;
            if (keystate[SDL_SCANCODE_S])
                move_delta -= cam.cam_forward;
            if (keystate[SDL_SCANCODE_A])
                move_delta -= cam.cam_right;
            if (keystate[SDL_SCANCODE_D])
                move_delta += cam.cam_right;

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

        // Camera
        cam.update_camera_vectors();
        cam.move_camera(move_delta, cam_speed, delta_time);

        // Draw Model

        for (auto &m : models)
        {
            rasterizer::process_model(m, cam, screen);
            m.fill_triangle_data();
            engine.draw_to_pixel(m, depth_buffer, pixels);
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

        // Find min/max depth (excluding infinity)
        float min_depth = std::numeric_limits<float>::infinity();
        float max_depth = 0.0f;
        for (float d : depth_buffer)
        {
            if (d < min_depth)
                min_depth = d;
            if (d > max_depth && d < std::numeric_limits<float>::infinity())
                max_depth = d;
        }

        // Copy Data from surface
        SDL_Rect rect{.x = 0, .y = 0, .w = width, .h = height};
        SDL_BlitSurface(draw_surface, nullptr, SDL_GetWindowSurface(window), &rect);

        SDL_UpdateWindowSurface(window);
    }

    // Clean up
    // SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
