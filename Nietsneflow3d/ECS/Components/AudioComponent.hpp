#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <vector>
#include <optional>

typedef unsigned int ALuint;

struct SoundElement
{
    ALuint m_sourceALID, m_bufferALID;
    bool m_toPlay;
};

struct AudioComponent : public ecs::Component
{
    AudioComponent()
    {
        muiTypeComponent = Components_e::AUDIO_COMPONENT;
    }
    std::vector<std::optional<SoundElement>> m_soundElements;
};
