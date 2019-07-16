#ifndef GRAPHICENGINE_H
#define GRAPHICENGINE_H

#include <functional>
#include <vector>
#include <string>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/Texture.hpp>

using pairStr_t = std::pair<std::string, std::string>;
using vectPairStr_t = std::vector<pairStr_t>;
using vectStr_t = std::vector<std::string>;

struct GLFWwindow;
class PictureData;

class GraphicEngine
{
private:
    std::vector<Shader> m_vectShader;
    std::vector<Texture> m_vectTexture;
    vectPairStr_t m_vectShaderPath;
    GLFWwindow* m_window = nullptr;
private:
    void loadShaderPathsFromFS();
    void initGLWindow();
    void initGLShader();
    void loadTextures(const vectStr_t &vectTextures);
public:
    GraphicEngine();
    void loadTextureAndSpriteData(const PictureData &pictureData);
};

#endif // GRAPHICENGINE_H
