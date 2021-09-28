#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <optional>
#include "AudioElement.hpp"

struct AudioComponent : public ecs::Component
{
    AudioComponent()
    {
        muiTypeComponent = Components_e::AUDIO_COMPONENT;
    }
    std::array<std::optional<AudioElement>, 3> m_soundElement;
};
