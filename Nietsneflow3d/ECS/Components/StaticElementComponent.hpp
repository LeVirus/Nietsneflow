#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct StaticElementComponent : public ecs::Component
{
    StaticElementComponent()
    {
        muiTypeComponent = Components_e::STATIC_ELEMENT_COMPONENT;
    }
    pairFloat_t m_inGameSpriteSize;
    LevelStaticElementType_e m_type;
    bool m_traversable;
    virtual ~StaticElementComponent() = default;
};
