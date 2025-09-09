#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "demos_app.hpp"
#include "terrain_gen.hpp"

constexpr int width = 2560;
constexpr int height = 1440;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    demo::demo_app app(width, height);

    if (!app.init_application())
        return -1;

    app.main_loop();

    app.clean_up();

    return 0;
}
