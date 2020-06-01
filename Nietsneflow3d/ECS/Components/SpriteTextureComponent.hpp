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
    pairFloat_t m_glFpsSize = {1.4f, 1.4f};
    virtual ~SpriteTextureComponent() = default;
};
