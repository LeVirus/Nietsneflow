#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <PictureData.hpp>
#include <functional>

using VectSpriteDataRef_t = std::vector<std::reference_wrapper<SpriteData>>;

struct WriteComponent : public ecs::Component
{
    WriteComponent()
    {
        muiTypeComponent = Components_e::WRITE_COMPONENT;
    }
    VectSpriteDataRef_t m_fontSpriteData;
    pairFloat_t m_upLeftPositionGL;
    uint8_t m_numTexture = static_cast<uint8_t>(Texture_e::FONT_T);
    virtual ~WriteComponent() = default;
};
