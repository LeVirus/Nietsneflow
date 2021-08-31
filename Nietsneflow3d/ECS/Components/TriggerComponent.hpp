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
        muiTypeComponent = Components_e::POSITION_VERTEX_COMPONENT;
    }
    std::vector<uint32_t> m_vectElementEntities;
    bool m_once;
    virtual ~TriggerComponent() = default;
};
