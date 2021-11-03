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
    std::pair<ALuint, ALuint> m_musicElement;
    SoundSystem *m_soundSystem = nullptr;
};

void printALError(ALenum error);
