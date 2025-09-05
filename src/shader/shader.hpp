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

    class lit_shader : public shader
    {
    public:
        rasterizer::vector3f light_direction = {0.0f, 0.0f, 0.0f};

        lit_shader(const rasterizer::vector3f &light_dir) : light_direction(std::move(light_dir)) {}

        rasterizer::vector3f shade(const rasterizer::vector3f &position,
                                   const rasterizer::vector3f &normal,
                                   const rasterizer::vector2f &tex_coord) const override
        {
            (void)position;
            (void)tex_coord;
            rasterizer::vector3f norm = normalized_vector(normal);
            float light_intensity = (rasterizer::dot(norm, light_direction) + 1.0f) * 0.5f;
            return vector3f{1.0f, 1.0f, 1.0f} * light_intensity;
        }
    };

    class lit_texture : public shader
    {
    public:
        rasterizer::texture texture;
        rasterizer::vector3f light_direction;
        float texture_scale = 1;

        lit_texture(const rasterizer::texture &tex, const rasterizer::vector3f &light_dir)
            : texture(tex), light_direction(light_dir) {}

        rasterizer::vector3f shade(const rasterizer::vector3f &position,
                                   const rasterizer::vector3f &normal,
                                   const rasterizer::vector2f &tex_coord) const override
        {
            (void)position;
            rasterizer::vector3f norm = normalized_vector(normal);
            float light_intensity = (rasterizer::dot(norm, light_direction) + 1.0f) * 0.5f;
            return texture.sample_texture(tex_coord.x * texture_scale, tex_coord.y * texture_scale) * light_intensity;
        }
    };
}
