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
    cleanUpAllBuffer();
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
void AudioEngine::cleanUpAllBuffer()
{
    if(m_musicElement)
    {
        // Destruction du tampon
        alDeleteBuffers(1, &m_musicElement->second);
    }
    std::map<std::string, ALuint>::const_iterator it = m_mapSoundEffect.begin();
    for(; it != m_mapSoundEffect.end(); ++it)
    {
        alDeleteBuffers(1, &it->second);
    }
    m_mapSoundEffect.clear();
}

//===================================================================
void AudioEngine::cleanUpBuffer(ALuint buffer)
{
    alDeleteBuffers(1, &buffer);
}

//===================================================================
std::optional<ALuint> AudioEngine::loadBufferFromFile(const std::string &filename,
                                                      bool soundEffect)
{
    std::string bufferFile = (soundEffect) ?
                LEVEL_RESSOURCES_DIR_STR + "Audio/SoundEffect/" + filename :
                LEVEL_RESSOURCES_DIR_STR + "Audio/Music/" + filename;
    SF_INFO fileInfos;
    SNDFILE *currentFile;
    currentFile = sf_open(bufferFile.c_str(), SFM_READ, &fileInfos);
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
    ALenum vv = alGetError();
    if(vv != AL_NO_ERROR)
    {
        switch (vv)
        {
        case AL_OUT_OF_MEMORY:
            std::cout << "AL_OUT_OF_MEMORY ";
            break;
        case AL_INVALID_OPERATION:
            std::cout << "AL_INVALID_OPERATION ";
            break;
        case AL_INVALID_VALUE:
            std::cout << "AL_INVALID_VALUE ";
            break;
        }
        std::cout << "ERROR on BUFFER LOADING\n";
        return {};
    }
    return memMusicBuffer;
}

//===================================================================
void AudioEngine::playMusic()
{
    if(m_musicElement)
    {
        m_soundSystem->play(m_musicElement->first);
    }
}

//===================================================================
void AudioEngine::clear()
{
    if(m_musicElement)
    {
        m_soundSystem->cleanUpSourceData(m_musicElement->first);
    }
    m_soundSystem->cleanUp();
    cleanUpAllBuffer();
}

//===================================================================
void AudioEngine::runIteration()
{
    m_soundSystem->execSystem();
}

//===================================================================
void AudioEngine::loadMusicFromFile(const std::string &filename)
{
    if(m_musicElement)
    {
        m_soundSystem->stop(m_musicElement->first);
    }
    else
    {
        m_musicElement = std::pair<ALuint, ALuint>();
    }
    std::optional<ALuint> memBuffer = loadBufferFromFile(filename, false);
    if(memBuffer)
    {
        m_musicElement->second = *memBuffer;
        m_musicElement->first = m_soundSystem->createSource(*memBuffer);
    }
}

//===================================================================
std::optional<ALuint> AudioEngine::loadSoundEffectFromFile(const std::string &filename)
{
    std::map<std::string, ALuint>::const_iterator it = m_mapSoundEffect.find(filename);
    if(it != m_mapSoundEffect.end())
    {
        return it->second;
    }
    std::optional<ALuint> memBuffer = loadBufferFromFile(filename, true);
    if(memBuffer)
    {
        m_mapSoundEffect.insert({filename, *memBuffer});
        return memBuffer;
    }
    return {};
}

//===================================================================
void AudioEngine::shutdownOpenAL()
{
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(m_context);
    alcCloseDevice(m_device);
}
