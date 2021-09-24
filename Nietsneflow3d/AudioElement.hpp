#pragma once

typedef unsigned int ALuint;

class AudioElement
{
public:
    AudioElement() = default;
    ~AudioElement();
    void conf();
    void play();
    inline void setBufferID(ALuint memSoundBuffer)
    {
        m_memSoundBuffer = memSoundBuffer;
    }
    void cleanUpSourceData();
private:
    ALuint m_soundSource, m_memSoundBuffer;
};
