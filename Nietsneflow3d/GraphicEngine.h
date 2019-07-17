#ifndef GRAPHICENGINE_H
#define GRAPHICENGINE_H

#include <functional>
#include <vector>
#include <string>
#include <PictureData.hpp>
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
    GroundCeilingData const *m_groundData = nullptr;
    GroundCeilingData const *m_ceilingData = nullptr;
private:
    void initGLWindow();
    void initGlad();
    void initGLShader();
    void loadTexturesPath(const vectStr_t &vectTextures);
    void loadGroundAndCeiling(const GroundCeilingData &groundData,
                              const GroundCeilingData &ceilingData);
    void loadSprites(const std::vector<SpriteData> &vectSprites);
public:
    GraphicEngine();
    void loadPictureData(const PictureData &pictureData);
};

#endif // GRAPHICENGINE_H
