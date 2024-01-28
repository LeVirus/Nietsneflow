#include "AudioEngine.hpp"
#include "sndfile.h"
#include <cassert>
#include <constants.hpp>
#include <string.h>
#include <iostream>
#include <algorithm>

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
    if(m_device)
    {
        std::cerr << "ERROR INITIALIZING OPENAL\n Error creating device\n";
        return;
    }
    m_context = alcCreateContext(m_device, nullptr);
    if(m_context)
    {
        std::cerr << "ERROR INITIALIZING OPENAL\n Error creating context\n";
        return;
    }
    /*ALCboolean res =*/ alcMakeContextCurrent(m_context);
    // assert(res);
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
    while(!m_vectMemBufferALID.empty())
    {
        cleanUpBuffer(m_vectMemBufferALID[0]);
    }
    if(m_musicElement)
    {
        alDeleteBuffers(1, &m_musicElement->second);
        m_musicElement = {};
    }
    m_mapSoundEffect.clear();
}

//===================================================================
void AudioEngine::cleanUpBuffer(ALuint buffer)
{
    std::vector<ALuint>::iterator it = std::find(m_vectMemBufferALID.begin(),
                                                 m_vectMemBufferALID.end(), buffer);
    assert(it != m_vectMemBufferALID.end());
    m_vectMemBufferALID.erase(it);
    alDeleteBuffers(1, &buffer);
}

//===================================================================
std::optional<ALuint> AudioEngine::loadBufferFromFile(const std::string &filename, bool soundEffect)
{
    std::string bufferFile = (soundEffect) ?
                RESSOURCES_DIR_STR + "Audio/SoundEffect/" + filename :
                RESSOURCES_DIR_STR + "Audio/Music/" + filename;
    SF_INFO fileInfos;
    SNDFILE *currentFile;
    currentFile = sf_open(bufferFile.c_str(), SFM_READ, &fileInfos);
    if(!currentFile)
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
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
    {
        printALError(error);
        assert(false);
    }
    // Remplissage avec les échantillons lus
    alBufferData(memMusicBuffer, format, &samples[0],
            nbSamples * sizeof(ALushort), sampleRate);
    // Vérification des erreurs
    error = alGetError();
    if(error != AL_NO_ERROR)
    {
        printALError(error);
        return {};
    }
    return memMusicBuffer;
}

//===================================================================
void AudioEngine::playMusic()
{
    if(m_musicElement)
    {
        updateMusicVolume(m_musicVolume);
        m_soundSystem->play(m_musicElement->first);
    }
}

//===================================================================
void AudioEngine::clearSourceAndBuffer()
{
    m_soundSystem->cleanUp();
    cleanUpAllBuffer();
}

//===================================================================
void AudioEngine::runIteration()
{
    m_soundSystem->execSystem();
}

//===================================================================
void AudioEngine::playEpilogueMusic()
{
    if(m_epilogueMusicFilename.empty() || m_epilogueMusicFilename == "None")
    {
        return;
    }
    m_soundSystem->stop(m_musicElement->first);
    loadMusicFromFile(m_epilogueMusicFilename);
    playMusic();
}

//===================================================================
void AudioEngine::loadMusicFromFile(const std::string &filename)
{
    std::optional<ALuint> memBuffer = loadBufferFromFile(filename, false);
    if(memBuffer)
    {
        m_musicElement = std::pair<ALuint, ALuint>();
        m_musicElement->second = *memBuffer;
        m_musicElement->first = m_soundSystem->createSource(*memBuffer);
        alSourcei(m_musicElement->first, AL_LOOPING, 1);
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
        m_vectMemBufferALID.push_back(*memBuffer);
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

//===================================================================
void printALError(ALenum error)
{
    switch (error)
    {
    case AL_OUT_OF_MEMORY:
        std::cout << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory ";
        break;
    case AL_INVALID_OPERATION:
        std::cout << "AL_INVALID_OPERATION: the requested operation is not valid ";
        break;
    case AL_INVALID_VALUE:
        std::cout << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function ";
        break;
    case AL_INVALID_NAME:
        std::cout << "AL_INVALID_NAME : a bad name (ID) was passed to an OpenAL function ";
        break;
    case AL_INVALID_ENUM:
        std::cout << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function ";
        break;
    }
    std::cout << "ERROR on BUFFER LOADING\n";
}
