#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

class Texture
{
private:
    std::string m_path;
    uint32_t m_textureNum;
private:
    void init();
    void load();
    std::string getPathExtension()const;
public:
    Texture(const std::string &path);
};

#endif // TEXTURE_H
