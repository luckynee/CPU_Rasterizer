#include <thread>
#include <atomic>

#include "rasterizer_engine.hpp"
#include "helper/obj_loader.hpp"

namespace rasterizer
{

    void rasterizer_engine::pre_renders()
    {
        clear_buffers();

        m_camera.update_camera_vectors();
        m_camera.move_camera(m_app->get_delta_time());
    }

    bool rasterizer_engine::is_model_visible(const model &m, const camera &cam)
    {
        // Compute bounding box in model space
        if (m.m_mesh.positions.empty())
            return false;

        rasterizer::vector3f min_bound = m.m_mesh.positions[0];
        rasterizer::vector3f max_bound = m.m_mesh.positions[0];
        for (const auto &v : m.m_mesh.positions)
        {
            min_bound.x = math::min(min_bound.x, v.x);
            min_bound.y = math::min(min_bound.y, v.y);
            min_bound.z = math::min(min_bound.z, v.z);
            max_bound.x = math::max(max_bound.x, v.x);
            max_bound.y = math::max(max_bound.y, v.y);
            max_bound.z = math::max(max_bound.z, v.z);
        }

        // Transform bounds to world space (apply model transform)
        rasterizer::vector3f min_world = transform_point(min_bound, m.model_transform);
        rasterizer::vector3f max_world = transform_point(max_bound, m.model_transform);

        // Simple near/far plane check (assuming camera looks along -Z)
        float near_plane = 0.01f;
        float far_plane = 1000.0f;

        if (max_world.z < cam.camera_transform.position.z + near_plane)
            return false;
        if (min_world.z > cam.camera_transform.position.z + far_plane)
            return false;

        // Optionally, add checks for X/Y screen bounds here

        return true;
    }

    //
    // Camera Functions
    //

    void rasterizer_engine::rotate_camera(int xrel, int yrel)
    {
        vector2f mouse_delta = vector2f{static_cast<float>(xrel), static_cast<float>(yrel)} / m_width * m_camera.mouse_sensitivity;
        m_camera.camera_transform.pitch = math::clamp(m_camera.camera_transform.pitch - mouse_delta.y, -math::to_radians(89.0f), math::to_radians(89.0f));
        m_camera.camera_transform.yaw += mouse_delta.x;
        m_camera.update_camera_vectors();
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
            std::fill(m_color_buffer, m_color_buffer + (m_width * m_height), to_uint32(m_clear_color));

        if (!m_depth_buffer.empty())
            std::fill(m_depth_buffer.begin(), m_depth_buffer.end(), std::numeric_limits<float>::infinity());
    }

    void rasterizer_engine::draw_to_pixel_tiled(const model &model,
                                                std::vector<float> &depth_buffer,
                                                std::uint32_t *pixels)
    {

        int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0)
        {
            num_threads = 4;
        }

        constexpr int TILE_SIZE = 64;

        std::vector<screen_tile> work_queue;

        for (int y = 0; y < m_height; y += TILE_SIZE)
        {
            for (int x = 0; x < m_width; x += TILE_SIZE)
            {
                work_queue.emplace_back(
                    x, y,
                    math::min(x + TILE_SIZE, m_width),
                    math::min(y + TILE_SIZE, m_height));
            }
        }

        std::atomic<int> work_index = 0;
        const int total_work_items = static_cast<int>(work_queue.size());

        std::vector<std::thread> threads;
        for (int t = 0; t < num_threads; ++t)
        {
            threads.emplace_back([&]()
                                 {
            while (true)
            {
                int index = work_index.fetch_add(1);

                if (index >= total_work_items)
                {
                    break; // No more work
                }

                const screen_tile &tile = work_queue[index];

                for (unsigned int i = 0; i < model.triangles_data.size(); ++i)
                {
                    const auto &triangle = model.triangles_data[i];

                    if (triangle.inv_depth.z <= 0 || triangle.inv_depth.y <= 0 || triangle.inv_depth.x <= 0)
                        continue;

                    if (triangle.maxX < tile.min_x || triangle.minX >= tile.max_x ||
                        triangle.maxY < tile.min_y || triangle.minY >= tile.max_y)
                    {
                        continue;
                    }

                    int x_start = math::max(tile.min_x, static_cast<int>(math::floor(triangle.minX)));
                    int x_end = math::min(tile.max_x, static_cast<int>(math::ceil(triangle.maxX)));
                    int y_start = math::max(tile.min_y, static_cast<int>(math::floor(triangle.minY)));
                    int y_end = math::min(tile.max_y, static_cast<int>(math::ceil(triangle.maxY)));

                    for (int y = y_start; y < y_end; ++y)
                    {
                        for (int x = x_start; x < x_end; ++x)
                        {
                            float px = static_cast<float>(x) + 0.5f;
                            float py = static_cast<float>(y) + 0.5f;
                            rasterizer::vector3f weight{0.0f, 0.0f, 0.0f};

                            if (!rasterizer::point_in_triangle(triangle.p0, triangle.p1, triangle.p2, px, py, weight))
                                continue;

                            float interpolated_z = 1.0f / (triangle.inv_depth.x * weight.x +
                                                           triangle.inv_depth.y * weight.y +
                                                           triangle.inv_depth.z * weight.z);
                            int idx = y * m_width + x;

                            if (interpolated_z >= depth_buffer[idx])
                                continue;

                            depth_buffer[idx] = interpolated_z;

                            vector3f position{
                                    triangle.p0.x * weight.x + triangle.p1.x * weight.y + triangle.p2.x * weight.z,
                                    triangle.p0.y * weight.x + triangle.p1.y * weight.y + triangle.p2.y * weight.z,
                                    interpolated_z
                            };
                            vector2f tex_coord = (triangle.tx * weight.x + triangle.ty * weight.y + triangle.tz * weight.z) * interpolated_z;
                            vector3f normal = (triangle.nx * weight.x + triangle.ny * weight.y + triangle.nz * weight.z) * interpolated_z;

                            if (model.shader_ptr)
                            {
                                
                                pixels[idx] = rasterizer::to_uint32(model.shader_ptr->shade(
                                    position, normal, tex_coord));
                            }
                            else
                            {
                                pixels[idx] = rasterizer::to_uint32(vector3f{1.0f, 0.0f, 1.0f});
                            }
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

    //
    // Derived Class Functions
    //

    void main_engine::setup_models()
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
            loaded_model2.mesh,
            loaded_model2.indices,
            floor_transform,
            m_shaders[0].get());
    }

    void main_engine::render_models()
    {
        for (auto &model : m_models)
        {
            if (!is_model_visible(model, m_camera))
                continue;

            // Process model
            process_model(model, m_camera, m_screen);

            model.fill_triangle_data();

            draw_to_pixel_tiled(model, m_depth_buffer, m_color_buffer);
        }
    }

    //
    // Global Function
    //

    void center_model(helper::model_data &model_data)
    {
        rasterizer::vector3f centroid{0.0f, 0.0f, 0.0f};
        for (const auto &v : model_data.mesh.positions)
        {
            centroid.x += v.x;
            centroid.z += v.z;
            centroid.y += v.y;
        }
        centroid.x /= model_data.mesh.positions.size();
        centroid.y /= model_data.mesh.positions.size();
        centroid.z /= model_data.mesh.positions.size();

        for (auto &v : model_data.mesh.positions)
        {
            v = v - centroid;
        }
    }

}
