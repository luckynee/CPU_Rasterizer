#include <algorithm>

#include "demos_app.hpp"

#include "core/helper/obj_loader.hpp"
#include "core/helper/math.hpp"
#include "core/shader/shader.hpp"

#include "terrain_gen.hpp"

namespace demo
{

    bool demo_app::init_application()
    {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
            return false;

        // Create window
        m_window = SDL_CreateWindow("Demo CPU Rasterizer",
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

        m_rasterizer_engine = std::make_unique<demo::demo_engine>(m_width, m_height, *this);

        setup_world();

        return true;
    }

    void demo_engine::setup_models()
    {
        // Make clear color sky blue
        m_clear_color = {135, 206, 235, 255};

        // Generate terrain
        rasterizer::mesh_data terrain_mesh = demo::generate_terrain(
            64,
            128.0f,
            {0.0f, 0.0f} // grid_center
        );

        std::vector<uint32_t> terrain_indices(terrain_mesh.positions.size());
        for (uint32_t i = 0; i < terrain_indices.size(); ++i)
            terrain_indices[i] = i;

        helper::model_data terrain{terrain_mesh, terrain_indices};

        rasterizer::center_model(terrain);

        // Create terrain transform
        rasterizer::transform terrain_transform;
        terrain_transform.scale = {1.0f, 1.0f, 1.0f};
        terrain_transform.position = {0.0f, -15.0f, 0.0f};

        // Terrrain shader
        demo::terrain_shader terrainShader(rasterizer::vector3f{0.0f, -1.0f, 0.0f});
        m_shaders.push_back(std::make_unique<demo::terrain_shader>(terrainShader));

        m_models.emplace_back(
            terrain.mesh,
            terrain.indices,
            terrain_transform,
            m_shaders[0].get());

        active_terrain.emplace_back(
            m_models.back(),
            rasterizer::vector2f{0.0f, 0.0f},
            0,
            0);
    }

    void demo_engine::render_models()
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

    void demo_engine::update_terrain_tiles(const rasterizer::vector3f &camera_pos, float tile_size, int resolution)
    {
    }
}