#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <functional>

using PairFloat_t = std::pair<float, float>;

struct TriggerComponent : public ecs::Component
{
    TriggerComponent()
    {
        muiTypeComponent = Components_e::TRIGGER_COMPONENT;
    }
    bool m_actionned = false;
    //class by shape
    std::map<uint32_t, std::vector<uint32_t>> m_mapElementEntities;
    virtual ~TriggerComponent() = default;
};
