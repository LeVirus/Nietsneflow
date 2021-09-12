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
    std::vector<uint32_t> m_vectElementEntities;
    virtual ~TriggerComponent() = default;
};
