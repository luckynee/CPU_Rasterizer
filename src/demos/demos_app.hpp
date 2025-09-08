#pragma once

#include "core_engine/application/application.hpp"
#include "core_engine/rasterizer/rasterizer_engine.hpp"

namespace demo
{
    struct terrain_tile
    {
        rasterizer::model model;
        rasterizer::vector2f grid_center;
        int grid_x, grid_y;

        terrain_tile() = default;

        terrain_tile(const rasterizer::model &m, const rasterizer::vector2f &center, int gx, int gy)
            : model(m), grid_center(center), grid_x(gx), grid_y(gy) {}
    };

    class demo_app : public application::application
    {
    public:
        using application::application;

        bool init_application() override;
    };

    class demo_engine : public rasterizer::rasterizer_engine
    {

    public:
        std::vector<terrain_tile> active_terrain;

        using rasterizer_engine::rasterizer_engine;

        void setup_models() override;

        void render_models() override;

        void update_terrain_tiles(const rasterizer::vector3f &camera_pos [[maybe_unused]],
                                  float tile_size [[maybe_unused]],
                                  int resolution [[maybe_unused]]);
    };

}
