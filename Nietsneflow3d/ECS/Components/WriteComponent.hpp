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
    float m_fontSize;
    pairFloat_t m_upLeftPositionGL;
    uint32_t m_numTexture;
    virtual ~WriteComponent() = default;
};
