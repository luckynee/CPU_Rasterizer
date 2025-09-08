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
        // // Generate terrain
        // rasterizer::mesh_data terrain_mesh = demo::generate_terrain(
        //     128,         // resolution (adjust as needed)
        //     20.0f,       // world_size (adjust as needed)
        //     {0.0f, 0.0f} // grid_center
        // );

        // std::vector<uint32_t> terrain_indices(terrain_mesh.positions.size());
        // for (uint32_t i = 0; i < terrain_indices.size(); ++i)
        //     terrain_indices[i] = i;

        // // Create terrain transform
        // rasterizer::transform terrain_transform;
        // terrain_transform.scale = {1.0f, 1.0f, 1.0f};
        // terrain_transform.position = {0.0f, 0.0f, 0.0f};

        // // Terrrain shader
        // demo::terrain_shader terrainShader(rasterizer::vector3f{0.0f, 1.0f, 0.0f});
        // m_shaders.push_back(std::make_unique<demo::terrain_shader>(terrainShader));

        // m_models.emplace_back(
        //     terrain_mesh.positions,
        //     terrain_indices,
        //     terrain_transform,
        //     m_shaders[0].get());

        return;
    }

}