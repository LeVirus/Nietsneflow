#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <PictureData.hpp>
#include <vector>

struct MemSpriteDataComponent : public ecs::Component
{
    MemSpriteDataComponent()
    {
        muiTypeComponent = Components_e::MEM_SPRITE_DATA_COMPONENT;
    }
    std::vector<SpriteData const *> m_vectSpriteData;
    virtual ~MemSpriteDataComponent() = default;
};
