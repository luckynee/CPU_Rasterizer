#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "core/application/application.hpp"
#include "core/helper/obj_loader.hpp"
#include "core/helper/math.hpp"
#include "core/rasterizer/model.hpp"
#include "core/rasterizer/rasterizer_engine.hpp"

constexpr int width = 2560;
constexpr int height = 1440;

int main()
{
    application::application app(width, height);

    if (!app.init_application())
        return -1;

    app.main_loop();

    app.clean_up();

    return 0;
}
