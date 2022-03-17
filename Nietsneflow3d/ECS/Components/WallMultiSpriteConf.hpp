#pragma once


#include <BaseECS/component.hpp>
#include <constants.hpp>

struct WallMultiSpriteConf : public ecs::Component
{
    WallMultiSpriteConf()
    {
        muiTypeComponent = Components_e::WALL_MULTI_SPRITE_CONF;
    }
    std::vector<float> m_time;
    virtual ~WallMultiSpriteConf() = default;
};
