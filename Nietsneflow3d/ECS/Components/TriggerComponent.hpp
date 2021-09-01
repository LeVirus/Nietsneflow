#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <functional>

using pairFloat_t = std::pair<float, float>;

struct TriggerComponent : public ecs::Component
{
    TriggerComponent()
    {
        muiTypeComponent = Components_e::TRIGGER_COMPONENT;
    }
    std::vector<uint32_t> m_vectElementEntities;
    virtual ~TriggerComponent() = default;
};
