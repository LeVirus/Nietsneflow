#pragma once


#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <memory>
#include <PictureData.hpp>



struct SpriteTextureComponent : public ecs::Component
{
    SpriteTextureComponent()
    {
        muiTypeComponent = Components_e::SPRITE_TEXTURE_COMPONENT;
    }
    SpriteData const *m_spriteData;
    pairFloat_t m_glFpsSize = {1.4f, 1.4f};
    std::unique_ptr<SpriteData> m_limitSpriteData;
    bool m_limitPointActive = false;
    virtual ~SpriteTextureComponent() = default;
};
