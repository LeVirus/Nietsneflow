#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

class Texture
{
private:
    std::string m_path, m_extension;
    uint32_t m_textureNum;
private:
    void initGLData();
    void load();
    void memPathExtension();
public:
    Texture(const std::string &path);
    inline const std::string getExtension()const
    {
        return m_extension;
    }
};

#endif // TEXTURE_H
