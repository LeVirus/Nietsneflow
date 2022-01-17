#pragma once

typedef	struct SNDFILE_tag	SNDFILE;
typedef unsigned int ALuint;
typedef int ALenum;

#include <AL/al.h>
#include <AL/alc.h>
#include <vector>
#include <map>
#include <string>
#include <optional>
#include "ECS/Systems/SoundSystem.hpp"

class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();
    void initOpenAL();
    void shutdownOpenAL();
    void playMusic();
    void clearSourceAndBuffer();
    void runIteration();
    void loadMusicFromFile(const std::string &filename);
    std::optional<ALuint> loadSoundEffectFromFile(const std::string &filename);
    inline SoundSystem *getSoundSystem()
    {
        return m_soundSystem;
    }
    inline void linkSystem(SoundSystem *soundSystem)
    {
        m_soundSystem = soundSystem;
    }
    inline void memPlayerEntity(uint32_t entityNum)
    {
        m_soundSystem->memPlayerEntity(entityNum);
    }
    inline void updateMusicVolume(uint32_t volume)
    {
        m_musicVolume = volume;
        alSourcef(m_musicElement->first, AL_GAIN, static_cast<float>(m_musicVolume) / 100.0f);
    }
    inline void updateEffectsVolume(uint32_t volume, bool playEffect = true)
    {
        m_soundSystem->updateEffectsVolume(volume, playEffect);
    }
    inline uint32_t getMusicVolume()const
    {
        return m_musicVolume;
    }
    inline uint32_t getEffectsVolume()const
    {
        return m_soundSystem->getEffectsVolume();
    }
    inline void memAudioMenuSound(ALuint source)
    {
        m_soundSystem->memAudioMenuSound(source);
    }
private:
    std::optional<ALuint> loadBufferFromFile(const std::string &filename, bool soundEffect);
    void updateDevices();
    void cleanUpAllBuffer();
    void cleanUpBuffer(ALuint buffer);
private:
    ALCdevice *m_device;
    ALCcontext *m_context;
    std::vector<std::string> m_vectDevices;
    std::vector<ALuint> m_vectMemBufferALID;
    std::map<std::string, ALuint> m_mapSoundEffect;
    //first = source, second = buffer
    std::optional<std::pair<ALuint, ALuint>> m_musicElement;
    SoundSystem *m_soundSystem = nullptr;
    uint32_t m_musicVolume = 100u;
};

void printALError(ALenum error);
