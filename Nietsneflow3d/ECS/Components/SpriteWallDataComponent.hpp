#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <memory>
#include <PictureData.hpp>

struct SpriteWallDataComponent : public ecs::Component
{
    SpriteWallDataComponent()
    {
        muiTypeComponent = Components_e::SPRITE_WALL_DATA_COMPONENT;
    }
    void fillWallContainer(const std::array<pairFloat_t, 4> &texturePosVertex)
    {
        if(!m_limitWallSpriteData)
        {
            m_limitWallSpriteData = std::make_unique<std::array<pairFloat_t, 8>>();
        }
        for(uint32_t i = 0; i < 8; ++i)
        {
            m_limitWallSpriteData->at(i) = texturePosVertex[i % 4];
        }
    }

    void reinitLimit()
    {
        if(m_limitWallPointActive)
        {
            m_limitWallPointActive = false;
            std::fill(m_limitWallSpriteData->begin(),
                      m_limitWallSpriteData->end(), pairFloat_t{EMPTY_VALUE, EMPTY_VALUE});
        }
    }

    std::unique_ptr<std::array<pairFloat_t, 8>> m_limitWallSpriteData;
    bool m_limitWallPointActive = false;
    virtual ~SpriteWallDataComponent() = default;
};
