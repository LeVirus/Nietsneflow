#ifndef SPRITETEXTUREDATA_HPP
#define SPRITETEXTUREDATA_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct SpriteData;

struct SpriteTextureComponent : public ecs::Component
{
    SpriteTextureComponent()
    {
        muiTypeComponent = Components_e::SPRITE_TEXTURE_COMPONENT;
    }
    SpriteData const *m_spriteData;
    virtual ~SpriteTextureComponent() = default;
};
#endif // SPRITETEXTUREDATA_HPP

