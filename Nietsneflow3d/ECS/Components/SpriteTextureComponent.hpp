#ifndef SPRITETEXTUREDATA_HPP
#define SPRITETEXTUREDATA_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct SpriteTextureComponent : public ecs::Component
{
    SpriteTextureComponent()
    {
        muiTypeComponent = Components_e::SPRITE_TEXTURE_COMPONENT;
    }
    uint8_t m_spriteNum;
    virtual ~SpriteTextureComponent() = default;
};
#endif // SPRITETEXTUREDATA_HPP

