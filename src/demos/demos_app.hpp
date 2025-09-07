#pragma once

#include "core/application/application.hpp"
#include "core/rasterizer/rasterizer_engine.hpp"

namespace demo
{
    class demo_app : public application::application
    {
    public:
        using application::application;

        bool init_application() override;
    };

    class demo_engine : public rasterizer::rasterizer_engine
    {
    public:
        using rasterizer_engine::rasterizer_engine;

        void setup_models() override;
    };
}