#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <functional>

struct LogComponent : public ecs::Component
{
    LogComponent()
    {
        muiTypeComponent = Components_e::LOG_COMPONENT;
    }
    std::string m_message;
};
