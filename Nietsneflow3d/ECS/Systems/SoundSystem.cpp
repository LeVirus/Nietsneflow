#include "SoundSystem.hpp"
#include <ECS/Components/AudioComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include "CollisionUtils.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <cassert>
#include <algorithm>

//===================================================================
SoundSystem::SoundSystem(NewComponentManager &newComponentManager) : m_newComponentManager(newComponentManager),
    m_componentsContainer(m_newComponentManager.getComponentsContainer())
{
    bAddComponentToSystem(Components_e::AUDIO_COMPONENT);
}

//===================================================================
void SoundSystem::execSystem()
{
    OptUint_t compNum;
    System::execSystem();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        compNum = m_newComponentManager.getComponentEmplacement(mVectNumEntity[i], Components_e::AUDIO_COMPONENT);
        assert(compNum);
        AudioComponent &audioComp = m_componentsContainer.m_vectAudioComp[*compNum];
        for(uint32_t j = 0; j < audioComp.m_soundElements.size(); ++j)
        {
            if(audioComp.m_soundElements[j] && audioComp.m_soundElements[j]->m_toPlay)
            {
                std::optional<float> volume = getVolumeFromDistance(mVectNumEntity[i], audioComp.m_maxDistance);
                if(volume)
                {
                    alSourcef(audioComp.m_soundElements[j]->m_sourceALID, AL_GAIN,
                              *volume * static_cast<float>(m_effectsVolume) / 100.0f);
                    play(audioComp.m_soundElements[j]->m_sourceALID);
                }
                audioComp.m_soundElements[j]->m_toPlay = false;
            }
        }
    }
}

//===================================================================
std::optional<float> SoundSystem::getVolumeFromDistance(uint32_t distantEntity, float maxDistance)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(m_playerEntity, Components_e::MAP_COORD_COMPONENT);
    assert(compNum);
    MapCoordComponent &playerMapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(distantEntity, Components_e::MAP_COORD_COMPONENT);
    if(!compNum)
    {
        return 1.0f;
    }
    MapCoordComponent &distantElementMapComp = m_componentsContainer.m_vectMapCoordComp[*compNum];
    float distance = getDistance(playerMapComp.m_absoluteMapPositionPX, distantElementMapComp.m_absoluteMapPositionPX);
    if(distance >= maxDistance)
    {
        return {};
    }
    return (maxDistance - distance) / maxDistance;
}

//===================================================================
SoundSystem::~SoundSystem()
{
    cleanUp();
}

//===================================================================
void SoundSystem::cleanUp()
{
    while(!m_vectSource.empty())
    {
        cleanUpSourceData(m_vectSource[0]);
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
    m_vectSource.push_back(source);
    return source;
}

//===================================================================
void SoundSystem::updateEffectsVolume(uint32_t volume, bool playEffect)
{
    m_effectsVolume = volume;
    alSourcef(m_sourceMenuAudio, AL_GAIN, static_cast<float>(m_effectsVolume) / 100.0f);
    if(playEffect)
    {
        play(m_sourceMenuAudio);
    }
}

//===================================================================
void SoundSystem::cleanUpSourceData(ALuint source)
{
    std::vector<ALuint>::iterator it = std::find(m_vectSource.begin(),
                                                 m_vectSource.end(), source);
    assert(it != m_vectSource.end());
    m_vectSource.erase(it);
    stop(source);
    // Destruction de la source
    alSourcei(source, AL_BUFFER, 0);
    alDeleteSources(1, &source);
}
