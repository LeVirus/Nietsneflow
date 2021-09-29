#include "SoundSystem.hpp"
#include <ECS/Components/AudioComponent.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <cassert>
#include "sndfile.h"

//===================================================================
SoundSystem::SoundSystem()
{
    bAddComponentToSystem(Components_e::AUDIO_COMPONENT);
}

//===================================================================
void SoundSystem::execSystem()
{
    if(mVectNumEntity.empty())
    {
        System::execSystem();
    }
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        AudioComponent *audioComp = stairwayToComponentManager().
                searchComponentByType<AudioComponent>(mVectNumEntity[i],
                                                      Components_e::AUDIO_COMPONENT);
        assert(audioComp);
        for(uint32_t j = 0; j < audioComp->m_soundElements.size(); ++j)
        {
            if(audioComp->m_soundElements[j].m_toPlay)
            {
                play(audioComp->m_soundElements[j].m_sourceALID);
                audioComp->m_soundElements[j].m_toPlay = false;
            }
        }
    }
}

//===================================================================
SoundSystem::~SoundSystem()
{
    cleanUp();
}

//===================================================================
void SoundSystem::cleanUp()
{
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        AudioComponent *audioComp = stairwayToComponentManager().
                searchComponentByType<AudioComponent>(mVectNumEntity[i],
                                                      Components_e::AUDIO_COMPONENT);
        assert(audioComp);
        for(uint32_t j = 0; j < audioComp->m_soundElements.size(); ++j)
        {
            cleanUpSourceData(audioComp->m_soundElements[j].m_sourceALID);
        }
    }
    mVectNumEntity.clear();
}

//===================================================================
void SoundSystem::play(ALuint source)
{
    alSourcePlay(source);
}

//===================================================================
void SoundSystem::stop(ALuint source)
{
    alSourceStop(source);
}

//===================================================================
ALuint SoundSystem::createSource(ALuint memSoundBuffer)
{
    ALuint source;
    // Création d'une source
    alGenSources(1, &source);
    // On attache le tampon contenant les échantillons audio à la source
    alSourcei(source, AL_BUFFER, memSoundBuffer);
    return source;
}

//===================================================================
void SoundSystem::cleanUpSourceData(ALuint source)
{
    stop(source);
    // Destruction de la source
    alSourcei(source, AL_BUFFER, 0);
    alDeleteSources(1, &source);
}
