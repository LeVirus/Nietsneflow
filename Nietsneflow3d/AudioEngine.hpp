#pragma once

typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;
typedef	struct SNDFILE_tag	SNDFILE;
typedef unsigned int ALuint;

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
    void clear();
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
private:
    std::optional<ALuint> loadBufferFromFile(const std::string &filename, bool soudEffect);
    void updateDevices();
    void cleanUpBuffer();
private:
    ALCdevice *m_device;
    ALCcontext *m_context;
    std::vector<std::string> m_vectDevices;
    std::map<std::string, ALuint> m_mapSoundEffect;
    std::optional<std::pair<ALuint, ALuint>> m_musicElement;
    SoundSystem *m_soundSystem = nullptr;
};
