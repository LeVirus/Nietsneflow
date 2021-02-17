#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <PictureData.hpp>

struct SpriteTextureComponent : public ecs::Component
{
    SpriteTextureComponent()
    {
        muiTypeComponent = Components_e::SPRITE_TEXTURE_COMPONENT;
    }
    SpriteData const *m_spriteData;
    pairFloat_t m_glFpsSize = {1.4f, 2.0f}, m_spriteLateralBound = {0.0f, 1.0f};
    bool m_boundActive = false;
    virtual ~SpriteTextureComponent() = default;
};
