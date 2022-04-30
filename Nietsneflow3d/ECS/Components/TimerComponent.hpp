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
    std::chrono::time_point<std::chrono::system_clock> m_clock;
    std::optional<uint32_t> m_timeIntervalOptional;
    uint32_t m_cycleCountA = 0, m_cycleCountB = 0, m_cycleCountC = 0, m_cycleCountD = 0;
    virtual ~TimerComponent() = default;
};

