#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <PictureData.hpp>
#include <vector>

/**
 * @brief The MemSpriteDataComponent struct stores all used sprite
 * in the case of animated entities.
 */
struct MemSpriteDataComponent : public ecs::Component
{
    MemSpriteDataComponent()
    {
        muiTypeComponent = Components_e::MEM_SPRITE_DATA_COMPONENT;
    }
    std::vector<SpriteData const *> m_vectSpriteData;
    virtual ~MemSpriteDataComponent() = default;
};
