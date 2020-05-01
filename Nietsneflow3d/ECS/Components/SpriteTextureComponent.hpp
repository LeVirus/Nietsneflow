#pragma once


#include <BaseECS/component.hpp>
#include <constants.hpp>

struct SpriteData;

struct SpriteTextureComponent : public ecs::Component
{
    SpriteTextureComponent()
    {
        muiTypeComponent = Components_e::SPRITE_TEXTURE_COMPONENT;
    }
    SpriteData const *m_spriteData;
    pairFloat_t m_glFpsSize = {1.0f, 1.0f};
    virtual ~SpriteTextureComponent() = default;
};
