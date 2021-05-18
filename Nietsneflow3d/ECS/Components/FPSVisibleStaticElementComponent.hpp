#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct FPSVisibleStaticElementComponent : public ecs::Component
{
    FPSVisibleStaticElementComponent()
    {
        muiTypeComponent = Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT;
    }
    pairFloat_t m_inGameSpriteSize;
    LevelStaticElementType_e m_levelElementType;
    virtual ~FPSVisibleStaticElementComponent() = default;
};
