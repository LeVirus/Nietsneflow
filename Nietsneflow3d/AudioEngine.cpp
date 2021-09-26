#include "AudioEngine.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include "sndfile.h"
#include <cassert>
#include <constants.hpp>
#include <string.h>
#include <iostream>

//===================================================================
AudioEngine::AudioEngine()
{

}

//===================================================================
AudioEngine::~AudioEngine()
{
    cleanUpBuffer();
    shutdownOpenAL();
}

//===================================================================
void AudioEngine::initOpenAL()
{
    m_device = alcOpenDevice(nullptr);
    assert(m_device);
    m_context = alcCreateContext(m_device, nullptr);
    assert(m_context);
    ALCboolean res = alcMakeContextCurrent(m_context);
    assert(res);
    updateDevices();
}


//===================================================================
void AudioEngine::updateDevices()
{
    //string null character divide
    const ALCchar* deviceList = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
    if(deviceList)
    {
        while(strlen(deviceList) > 0)
        {
            m_vectDevices.push_back(deviceList);
            deviceList += strlen(deviceList) + 1;
        }
    }
}

//===================================================================
void AudioEngine::cleanUpBuffer()
{
    if(m_memMusicBuffer)
    {
        // Destruction du tampon
        alDeleteBuffers(1, &(*m_memMusicBuffer));
        m_memMusicBuffer = {};
    }
}

//===================================================================
std::optional<ALuint> AudioEngine::loadBufferFromFile(const std::string &filename)
{
    std::string musicFile = LEVEL_RESSOURCES_DIR_STR + "Audio/Music/" + filename;
    SF_INFO fileInfos;
    SNDFILE *currentFile;
    currentFile = sf_open(musicFile.c_str(), SFM_READ, &fileInfos);
    if (!currentFile)
    {
        return {};
    }
    ALsizei nbSamples  = static_cast<ALsizei>(fileInfos.channels * fileInfos.frames);
    ALsizei sampleRate = static_cast<ALsizei>(fileInfos.samplerate);
    // Lecture des échantillons audio au format entier 16 bits signé (le plus commun)
    std::vector<ALshort> samples(nbSamples);
    if(sf_read_short(currentFile, &samples[0], nbSamples) < nbSamples)
    {
        return {};
    }
    sf_close(currentFile);
    // Détermination du format en fonction du nombre de canaux
    ALenum format;
    switch (fileInfos.channels)
    {
        case 1:
        format = AL_FORMAT_MONO16;
        break;
        case 2:
        format = AL_FORMAT_STEREO16;
        break;
        default:
        return {};
    }
    ALuint memMusicBuffer;
    // Création du tampon OpenAL
    alGenBuffers(1, &memMusicBuffer);
    // Remplissage avec les échantillons lus
    alBufferData(memMusicBuffer, format, &samples[0],
            nbSamples * sizeof(ALushort), sampleRate);
    // Vérification des erreurs
    if(alGetError() != AL_NO_ERROR)
    {
        std::cout << "ERROR on BUFFER LOADING\n";
        return {};
    }
    return memMusicBuffer;
}

//===================================================================
void AudioEngine::playMusic()
{
    m_musicElement.play();
}

//===================================================================
void AudioEngine::clear()
{
    m_musicElement.cleanUpSourceData();
}

//===================================================================
void AudioEngine::loadMusic(const std::string &filename)
{
    if(m_memMusicBuffer)
    {
        m_musicElement.stop();
        cleanUpBuffer();
    }
    std::optional<ALuint> memBuffer = loadBufferFromFile(filename);
    if(memBuffer)
    {
        m_memMusicBuffer = *memBuffer;
        m_musicElement.setBufferID(*memBuffer);
        m_musicElement.conf();
    }
}

//===================================================================
void AudioEngine::runIteration(bool gamePaused)
{

}

//===================================================================
void AudioEngine::shutdownOpenAL()
{
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(m_context);
    alcCloseDevice(m_device);
}
