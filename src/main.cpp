#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "application/application.hpp"
#include "helper/obj_loader.hpp"
#include "helper/math.hpp"
#include "rasterizer/model.hpp"
#include "rasterizer/rasterizer_engine.hpp"

// TODO -> change this later, only placeholder
constexpr int width = 800;
constexpr int height = 600;

int main()
{
    application::application app(width, height);

    if (!app.init_application())
        return -1;

    app.main_loop();

    app.clean_up();

    return 0;
}
