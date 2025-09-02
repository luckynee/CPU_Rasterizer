#pragma once

#include "rasterizer/types.hpp"

namespace rasterizer
{
    class shader
    {
    public:
        virtual ~shader() = default;

        virtual rasterizer::vector3f shade(const rasterizer::vector3f &position,
                                           const rasterizer::vector3f &normal,
                                           const rasterizer::vector2f &tex_coord) const = 0;
    };

    class texture_shader : public shader
    {
    public:
        rasterizer::texture texture;

        texture_shader(const rasterizer::texture &tex) : texture(tex) {}

        rasterizer::vector3f shade(const rasterizer::vector3f &position,
                                   const rasterizer::vector3f &normal,
                                   const rasterizer::vector2f &tex_coord) const override
        {
            (void)position;
            (void)normal;
            return texture.sample_texture(tex_coord.x, tex_coord.y);
        }
    };
}
