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
    std::string m_str;
    pairFloat_t m_upLeftPositionGL;
    Texture_e m_numTexture = Texture_e::FONT_T;
    virtual ~WriteComponent() = default;
};
