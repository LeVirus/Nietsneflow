#ifndef GRAPHICENGINE_H
#define GRAPHICENGINE_H

#include <functional>
#include <vector>
#include <string>
#include <OpenGLUtils/Shader.hpp>

using pairStr_t = std::pair<std::string, std::string>;
using vectPairStr_t = std::vector<pairStr_t>;

struct GLFWwindow;

class GraphicEngine
{
private:
    std::vector<Shader> m_vectShader;
    vectPairStr_t m_vectShaderPath;
    GLFWwindow* m_window = nullptr;
private:
    void initGLWindow();
    void initGLShader();
    void loadShaderFromFS();
public:
    GraphicEngine();
};

#endif // GRAPHICENGINE_H
