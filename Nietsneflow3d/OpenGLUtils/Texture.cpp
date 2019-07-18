#include <OpenGLUtils/Texture.hpp>
//#ifndef __glad_h_
//#define __glad_h_
#include <glad/glad.h>
//#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//===================================================================
Texture::Texture(const std::string &path) : m_path(path)
{
    init();
    load();
}

//===================================================================
void Texture::init()
{
    glGenTextures(1, &m_textureNum);
    glBindTexture(GL_TEXTURE_2D, m_textureNum);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

//===================================================================
void Texture::load()
{
    int width, height, nrChannels;
    uint8_t *data = stbi_load(m_path.c_str(), &width, &height, &nrChannels, 0);
    std::string ext = getPathExtension();
    uint32_t targetRGB;
    if(ext == "png")
    {
        targetRGB = GL_RGBA;
    }
    else
    {
        targetRGB = GL_RGB;
    }
    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, targetRGB, width, height, 0,
                     targetRGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        assert("Texture failed to load.");
    }
}

//===================================================================
std::string Texture::getPathExtension() const
{
    size_t size = m_path.size();

    std::string substr = m_path.substr(size - 3);
    if(substr != "jpg" && substr != "png")
    {
        assert("Extension not supported.");
    }
    return substr;
}

