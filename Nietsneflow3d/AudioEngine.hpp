#pragma once

typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;
typedef	struct SNDFILE_tag	SNDFILE;
typedef unsigned int ALuint;

#include <vector>
#include <string>
#include <optional>
#include "AudioElement.hpp"

class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();
    void initOpenAL();
    void runIteration(bool gamePaused);
    void shutdownOpenAL();
    void playMusic();
    void clear();
    void loadMusic(const std::string &filename);
private:
    std::optional<ALuint> loadFile(const std::string &filename);
    void updateDevices();
    void cleanUpBuffer();
private:
    ALCdevice *m_device;
    ALCcontext *m_context;
    std::vector<std::string> m_vectDevices;
    ALuint m_memMusicBuffer;
    AudioElement m_musicElement;
};
