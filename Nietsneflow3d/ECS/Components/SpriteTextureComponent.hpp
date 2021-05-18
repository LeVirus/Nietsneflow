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
    virtual ~SpriteTextureComponent() = default;
};
