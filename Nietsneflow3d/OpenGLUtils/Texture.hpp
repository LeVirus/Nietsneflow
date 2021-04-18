#pragma once

#include <string>

class Texture
{
public:
    Texture(const std::string &path);
    void bind();
    void unbind();
private:
    void initGLData();
    void load();
    void memPathExtension();
private:
    std::string m_path, m_extension;
    uint32_t m_textureNum;
};
