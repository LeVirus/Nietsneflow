#include "AudioElement.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <iostream>
#include <cassert>
#include "sndfile.h"

//===================================================================
AudioElement::~AudioElement()
{
    cleanUpSourceData();
}

//===================================================================
void AudioElement::conf()
{
    stop();
    if(!m_soundSourceID)
    {
        ALuint source;
        // Création d'une source
        alGenSources(1, &source);
        m_soundSourceID = source;
    }
    // On attache le tampon contenant les échantillons audio à la source
    alSourcei(*m_soundSourceID, AL_BUFFER, *m_memSoundBufferID);
}

//===================================================================
void AudioElement::play()
{
    if(m_soundSourceID)
    {
        alSourcePlay(*m_soundSourceID);
    }
}

//===================================================================
void AudioElement::stop()
{
    if(m_soundSourceID)
    {
        alSourceStop(*m_soundSourceID);
    }
}

//===================================================================
void AudioElement::cleanUpSourceData()
{
    if(m_soundSourceID)
    {
        stop();
        // Destruction de la source
        alSourcei(*m_soundSourceID, AL_BUFFER, 0);
        alDeleteSources(1, &(*m_soundSourceID));
        m_soundSourceID = {};
    }
}
