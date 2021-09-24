#include "AudioElement.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include "sndfile.h"

//===================================================================
AudioElement::~AudioElement()
{
    cleanUpSourceData();
}

//===================================================================
void AudioElement::conf()
{
    // Création d'une source
    alGenSources(1, &m_soundSource);
    // On attache le tampon contenant les échantillons audio à la source
    alSourcei(m_soundSource, AL_BUFFER, m_memSoundBuffer);
}

//===================================================================
void AudioElement::play()
{
    alSourcePlay(m_soundSource);
}

//===================================================================
void AudioElement::cleanUpSourceData()
{
    // Destruction de la source
    alSourcei(m_soundSource, AL_BUFFER, 0);
    alDeleteSources(1, &m_soundSource);
}
