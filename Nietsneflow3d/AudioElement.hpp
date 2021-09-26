#pragma once

#include <optional>

typedef unsigned int ALuint;

class AudioElement
{
public:
    AudioElement() = default;
    ~AudioElement();
    void conf();
    void play();
    void stop();
    inline void setBufferID(ALuint memSoundBuffer)
    {
        m_memSoundBufferID = memSoundBuffer;
    }
    void cleanUpSourceData();
private:
    std::optional<ALuint> m_soundSourceID, m_memSoundBufferID;
};
