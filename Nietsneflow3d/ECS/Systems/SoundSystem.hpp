#pragma once

#include "constants.hpp"
#include <includesLib/BaseECS/system.hpp>

typedef unsigned int ALuint;

class SoundSystem : public ecs::System
{
public:
    SoundSystem();
    ~SoundSystem();
    void execSystem()override;
    void play(ALuint source);
    void cleanUpSourceData(ALuint source);
    void cleanUp();
    void stop(ALuint source);
    ALuint createSource(ALuint memSoundBuffer);
private:
    std::vector<ALuint> m_vectSource;
};
