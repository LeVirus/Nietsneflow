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
    void fillWallContainer()
    {
        //!!!IMPORTANT COPY BASE SPRITE DATA
        if(!m_limitWallSpriteData)
        {
            m_limitWallSpriteData = std::make_unique<std::array<std::pair<float, float>, 8>>();
        }
        for(uint32_t i = 0; i < 8; ++i)
        {
            m_limitWallSpriteData->at(i) = m_spriteData->m_texturePosVertex[i % 4];
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

    SpriteData const *m_spriteData;
    pairFloat_t m_glFpsSize = {1.4f, 1.4f};
    std::unique_ptr<std::array<std::pair<float, float>, 8>> m_limitWallSpriteData;
    bool m_limitWallPointActive = false;
    virtual ~SpriteTextureComponent() = default;
};
