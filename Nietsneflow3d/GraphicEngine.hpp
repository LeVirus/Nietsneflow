#ifndef GRAPHICENGINE_H
#define GRAPHICENGINE_H

#include <functional>
#include <vector>
#include <string>
#include <PictureData.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
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
    GLFWwindow* m_window = nullptr;
    std::vector<Shader> m_vectShader;
    //PictureData
    std::vector<Texture> m_vectTexture;
    std::vector<SpriteData> const *m_ptrSpriteData = nullptr;
    //Systems
    ColorDisplaySystem *m_colorSystem = nullptr;
private:
    void initGLWindow();
    void initGlad();
    void initGLShader();
    void setShaderToLocalSystems();
    void loadTexturesPath(const vectStr_t &vectTextures);
    void loadGroundAndCeiling(const GroundCeilingData &groundData,
                              const GroundCeilingData &ceilingData);
    void loadSprites(const std::vector<SpriteData> &vectSprites);
public:
    GraphicEngine();
    void confSystems();
    void loadPictureData(const PictureData &pictureData);
    void runIteration();
    bool windowShouldClose();
    void linkSystems(ColorDisplaySystem *system);
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#endif // GRAPHICENGINE_H
