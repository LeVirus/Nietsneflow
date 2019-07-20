#ifndef COLORDISPLAYSYSTEM_H
#define COLORDISPLAYSYSTEM_H

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>

class ColorDisplaySystem : public ecs::System
{
private:
    Shader *m_shader;
    std::vector<float> m_vectVertex;
    GLFWwindow* m_window = nullptr;
private:
    void fillVertexFromEntities();
    void drawVertex();
public:
    ColorDisplaySystem();
    void setUsedComponents();
    void execSystem()override;
    void setShader(Shader &shader);
    void display()const;
    inline void setWindow(GLFWwindow* window){window = m_window;}
};

#endif // COLORDISPLAYSYSTEM_H
