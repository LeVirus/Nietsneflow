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
    inline void memPlayerEntity(uint32_t entityNum)
    {
        m_playerEntity = entityNum;
    }
private:
    std::optional<float> getVolumeFromDistance(uint32_t distantEntity, float maxDistance);
private:
    uint32_t m_playerEntity;
    std::vector<ALuint> m_vectSource;
};
