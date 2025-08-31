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
        myModel.triangles.clear();
        for (unsigned int i = 0; i < myModel.vertices.size(); i += 3)
        {
            rasterizer::vector3f v0 = rasterizer::vertex_to_screen(myModel.vertices[i], modelTransform, {width, height}, fov);
            rasterizer::vector3f v1 = rasterizer::vertex_to_screen(myModel.vertices[i + 1], modelTransform, {width, height}, fov);
            rasterizer::vector3f v2 = rasterizer::vertex_to_screen(myModel.vertices[i + 2], modelTransform, {width, height}, fov);

            rasterizer::vector2f p0 = static_cast<rasterizer::vector2f>(v0);
            rasterizer::vector2f p1 = static_cast<rasterizer::vector2f>(v1);
            rasterizer::vector2f p2 = static_cast<rasterizer::vector2f>(v2);

            // Triangle bounds
            float minX = rasterizer::min(rasterizer::min(p0.x, p1.x), p2.x);
            float minY = rasterizer::min(rasterizer::min(p0.y, p1.y), p2.y);
            float maxX = rasterizer::max(rasterizer::max(p0.x, p1.x), p2.x);
            float maxY = rasterizer::max(rasterizer::max(p0.y, p1.y), p2.y);

            myModel.triangles.emplace_back(rasterizer::triangle_data{v0, v1, v2, p0, p1, p2, minX, maxX, minY, maxY});
        }

        // Draw each pixel
        for (unsigned int i = 0; i < myModel.triangles.size(); ++i)
        {
            const auto &triangle = myModel.triangles[i];

            // Rasterize triangle within its bounding box
            int x_start = rasterizer::clamp(static_cast<int>(std::floor(triangle.minX)), 0, width - 1);
            int x_end = rasterizer::clamp(static_cast<int>(std::ceil(triangle.maxX)), 0, width - 1);
            int y_start = rasterizer::clamp(static_cast<int>(std::floor(triangle.minY)), 0, height - 1);
            int y_end = rasterizer::clamp(static_cast<int>(std::ceil(triangle.maxY)), 0, height - 1);

            for (int y = y_start; y <= y_end; ++y)
            {
                for (int x = x_start; x <= x_end; ++x)
                {
                    rasterizer::vector2f p{static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f}; // center of pixel
                    rasterizer::vector3f weight{0.0f, 0.0f, 0.0f};

                    if (rasterizer::point_in_triangle(triangle.v2a, triangle.v2b, triangle.v2c, p, weight))
                    {
                        // Interpolate depth
                        float interpolated_z = weight.x * triangle.v3a.z + weight.y * triangle.v3b.z + weight.z * triangle.v3c.z;

                        int idx = y * width + x;
                        if (interpolated_z < depth_buffer[idx])
                        {
                            depth_buffer[idx] = interpolated_z;

                            rasterizer::color4ub color = rasterizer::to_color4ub(myModel.triangleCols[i]);
                            pixels[idx] = rasterizer::to_uint32(color);
                        }
                    }
                }
            }
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
        //         {
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
            std::cout << "FPS: " << fps << "\n";
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
