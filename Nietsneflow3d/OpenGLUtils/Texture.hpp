#pragma once

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
    void bind();
    void unbind();
};
