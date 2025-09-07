#include "demos_app.hpp"

#include "core/helper/obj_loader.hpp"
#include "core/helper/math.hpp"
#include "core/shader/shader.hpp"

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
        // Create Shader
        m_shaders.push_back(std::make_unique<rasterizer::texture_shader>("../resource/textures/uvGrid.bytes"));

        // Load Model
        helper::model_data loaded_model2 = helper::load_obj("../resource/model/floor.obj");
        rasterizer::center_model(loaded_model2);

        // Create Transform
        rasterizer::transform floor_transform;
        floor_transform.scale = {3.0f, 1.0f, 3.0f};
        floor_transform.position = {0.0f, -2.0f, 0.0f};

        // Create Model
        m_models.emplace_back(
            loaded_model2.vertices,
            loaded_model2.indices,
            floor_transform,
            nullptr);

        return;
    }

}