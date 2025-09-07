#include <thread>

#include "rasterizer_engine.hpp"
#include "helper/obj_loader.hpp"

namespace rasterizer
{

    void rasterizer_engine::setup_models()
    {
        // Create Shader
        m_shaders.push_back(std::make_unique<texture_shader>("../resource/textures/uvGrid.bytes"));

        // Load Model
        helper::model_data loaded_model2 = helper::load_obj("../resource/model/floor.obj");
        center_model(loaded_model2);

        // Create Transform
        transform floor_transform;
        floor_transform.scale = {3.0f, 1.0f, 3.0f};
        floor_transform.position = {0.0f, -2.0f, 0.0f};

        // Create Model
        m_models.emplace_back(
            loaded_model2.vertices,
            loaded_model2.indices,
            floor_transform,
            m_shaders[0].get());
    }

    void rasterizer_engine::pre_renders()
    {
        clear_buffers();

        m_camera.update_camera_vectors();
        m_camera.move_camera(m_app->get_delta_time());
    }

    void rasterizer_engine::render_models()
    {
        for (auto &model : m_models)
        {
            // Process model
            process_model(model, m_camera, m_screen);

            model.fill_triangle_data();

            draw_to_pixel_multithreaded(model, m_depth_buffer, m_color_buffer);
            // draw_to_pixel(model, m_depth_buffer, m_color_buffer);
        }
    }

    //
    // Camera Functions
    //

    void rasterizer_engine::rotate_camera(int xrel, int yrel)
    {
        vector2f mouse_delta = vector2f{static_cast<float>(xrel), static_cast<float>(yrel)} / m_width * m_camera.mouse_sensitivity;
        m_camera.camera_transform.pitch = math::clamp(m_camera.camera_transform.pitch - mouse_delta.y, -math::to_radians(89.0f), math::to_radians(89.0f));
        m_camera.camera_transform.yaw += mouse_delta.x;
    }

    void rasterizer_engine::move_camera(const Uint8 *key_state)
    {
        m_camera.move_delta.reset_to_zero();

        if (key_state[SDL_SCANCODE_W])
            m_camera.move_delta += m_camera.cam_forward;
        if (key_state[SDL_SCANCODE_S])
            m_camera.move_delta -= m_camera.cam_forward;
        if (key_state[SDL_SCANCODE_A])
            m_camera.move_delta -= m_camera.cam_right;
        if (key_state[SDL_SCANCODE_D])
            m_camera.move_delta += m_camera.cam_right;
    }

    //
    // Private Methods
    //

    void rasterizer_engine::clear_buffers()
    {
        if (m_color_buffer)
            std::fill(m_color_buffer, m_color_buffer + (m_width * m_height), 0);

        if (!m_depth_buffer.empty())
            std::fill(m_depth_buffer.begin(), m_depth_buffer.end(), std::numeric_limits<float>::infinity());
    }

    void rasterizer_engine::draw_to_pixel_multithreaded(const model &model,
                                                        std::vector<float> &depth_buffer,
                                                        std::uint32_t *pixels)
    {
        int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0)
            num_threads = 4;

        int row_per_thread = m_height / num_threads;
        std::vector<std::thread> threads;

        for (int t = 0; t < num_threads; ++t)
        {
            int y_start = t * row_per_thread;
            int y_end = (t == num_threads - 1) ? m_height - 1 : (y_start + row_per_thread - 1);

            threads.emplace_back([=, &model, &depth_buffer, &pixels, this]()
                                 {
                for(unsigned int i =0 ; i< model.triangles_data.size(); ++i){
                    const auto &triangle = model.triangles_data[i];

                    if (triangle.inv_depth.z <= 0 || triangle.inv_depth.y <= 0 || triangle.inv_depth.x <= 0)
                        continue;
                    if (triangle.maxX < 0 || triangle.minX > m_screen.x - 1 ||
                        triangle.maxY < 0 || triangle.minY > m_screen.y - 1)
                        continue;

                    int x_start = math::clamp(static_cast<int>(math::floor(triangle.minX)), 0, static_cast<int>(m_screen.x) - 1);
                    int x_end = math::clamp(static_cast<int>(math::ceil(triangle.maxX)), 0, static_cast<int>(m_screen.x) - 1);
                    int tri_y_start = math::max(y_start, math::clamp(static_cast<int>(math::floor(triangle.minY)), 0, static_cast<int>(m_screen.y) - 1));
                    int tri_y_end = math::min(y_end, math::clamp(static_cast<int>(math::ceil(triangle.maxY)), 0, static_cast<int>(m_screen.y) - 1));
    

                    for (int y = tri_y_start; y <= tri_y_end; ++y)
                    {
                        for (int x = x_start; x <= x_end; ++x)
                        {
                            float px = static_cast<float>(x) + 0.5f;
                            float py = static_cast<float>(y) + 0.5f;
                            rasterizer::vector3f weight{0.0f, 0.0f, 0.0f};

                            if (!rasterizer::point_in_triangle(triangle.p0, triangle.p1, triangle.p2, px, py, weight))
                                continue;

                            float interpolated_z = 1.0f / (triangle.inv_depth.x * weight.x +
                                                           triangle.inv_depth.y * weight.y +
                                                           triangle.inv_depth.z * weight.z);
                            int idx = y * m_screen.x + x;

                            if (interpolated_z >= depth_buffer[idx])
                                continue;

                            vector2f tex_coord = (triangle.tx * weight.x +
                                                  triangle.ty * weight.y +
                                                  triangle.tz * weight.z) *
                                                 interpolated_z;

                            depth_buffer[idx] = interpolated_z;

                            vector3f normal =
                                (triangle.nx * weight.x +
                                 triangle.ny * weight.y +
                                 triangle.nz * weight.z) *
                                interpolated_z;

                            if (model.shader_ptr)
                            {
                                pixels[idx] = rasterizer::to_uint32(model.shader_ptr->shade(
                                    rasterizer::vector3f{0, 0, 0},
                                    normal,
                                    tex_coord));
                            }
                            else
                            {
                                pixels[idx] = rasterizer::to_uint32(model.triangle_colors[i]);
                            }
                        }
                    }
                        
                } });
        }

        for (auto &th : threads)
        {
            if (th.joinable())
                th.join();
        }
    }

    void rasterizer_engine::draw_to_pixel(
        const model &model,
        std::vector<float> &depth_buffer,
        std::uint32_t *pixels)
    {
        for (unsigned int i = 0; i < model.triangles_data.size(); ++i)
        {
            const auto &triangle = model.triangles_data[i];

            // Skip triangles that are completely outside the screen
            if (triangle.inv_depth.z <= 0 || triangle.inv_depth.y <= 0 || triangle.inv_depth.x <= 0)
                continue;

            if (triangle.maxX < 0 || triangle.minX > m_screen.x - 1 ||
                triangle.maxY < 0 || triangle.minY > m_screen.y - 1)
                continue; // Triangle is completely outside the screen

            // Rasterize triangle within its bounding box
            int x_start = math::clamp(static_cast<int>(math::floor(triangle.minX)), 0, static_cast<int>(m_screen.x) - 1);
            int x_end = math::clamp(static_cast<int>(math::ceil(triangle.maxX)), 0, static_cast<int>(m_screen.x) - 1);
            int y_start = math::clamp(static_cast<int>(math::floor(triangle.minY)), 0, static_cast<int>(m_screen.y) - 1);
            int y_end = math::clamp(static_cast<int>(math::ceil(triangle.maxY)), 0, static_cast<int>(m_screen.y) - 1);

            for (int y = y_start; y <= y_end; ++y)
            {
                for (int x = x_start; x <= x_end; ++x)
                {
                    float px = static_cast<float>(x) + 0.5f;
                    float py = static_cast<float>(y) + 0.5f;
                    rasterizer::vector3f weight{0.0f, 0.0f, 0.0f};

                    if (!rasterizer::point_in_triangle(triangle.p0, triangle.p1, triangle.p2, px, py, weight))
                        continue;

                    float interpolated_z = 1.0f / (triangle.inv_depth.x * weight.x +
                                                   triangle.inv_depth.y * weight.y +
                                                   triangle.inv_depth.z * weight.z);
                    int idx = y * m_screen.x + x;

                    if (interpolated_z >= depth_buffer[idx])
                        continue;

                    // Interpolate tex coord
                    vector2f tex_coord = (triangle.tx * weight.x +
                                          triangle.ty * weight.y +
                                          triangle.tz * weight.z) *
                                         interpolated_z;

                    depth_buffer[idx] = interpolated_z;

                    // Interpolate normal
                    vector3f normal =
                        (triangle.nx * weight.x +
                         triangle.ny * weight.y +
                         triangle.nz * weight.z) *
                        interpolated_z;

                    if (model.shader_ptr)
                    {
                        pixels[idx] = rasterizer::to_uint32(model.shader_ptr->shade(
                            rasterizer::vector3f{0, 0, 0}, // position not used
                            normal,
                            tex_coord));
                    }
                    else
                    {
                        pixels[idx] = rasterizer::to_uint32(model.triangle_colors[i]);
                    }
                }
            }
        }
    }

    //
    // Global Function
    //

    void center_model(helper::model_data &model_data)
    {
        rasterizer::vector3f centroid{0.0f, 0.0f, 0.0f};
        for (const auto &v : model_data.vertices)
        {
            centroid.x += v.position.x;
            centroid.y += v.position.y;
            centroid.z += v.position.z;
        }
        centroid.x /= model_data.vertices.size();
        centroid.y /= model_data.vertices.size();
        centroid.z /= model_data.vertices.size();

        for (auto &v : model_data.vertices)
        {
            v.position = v.position - centroid;
        }
    }

}
