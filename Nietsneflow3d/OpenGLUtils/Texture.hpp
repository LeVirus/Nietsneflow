#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

class Texture
{
private:
    std::string m_path;
    uint32_t m_textureNum;
public:
    Texture(const std::string &path);
    void init();
};

#endif // TEXTURE_H
