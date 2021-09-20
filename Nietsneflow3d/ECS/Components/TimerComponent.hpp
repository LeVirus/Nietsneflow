#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <chrono>
#include <ctime>

struct TimerComponent : public ecs::Component
{
    TimerComponent()
    {
        muiTypeComponent = Components_e::TIMER_COMPONENT;
    }
    std::chrono::time_point<std::chrono::system_clock> m_clockA, m_clockB, m_clockC, m_clockD;
    virtual ~TimerComponent() = default;
};

