#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <optional>
#include <bitset>

typedef unsigned int ALuint;

struct AudioComponent : public ecs::Component
{
    AudioComponent()
    {
        muiTypeComponent = Components_e::AUDIO_COMPONENT;
    }
    //first source second buffer
    std::array<std::optional<std::pair<ALuint, ALuint>>, 3> m_soundElement;
    std::bitset<3> m_elementToPlay;
};
