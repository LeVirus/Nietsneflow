#ifndef COLORDISPLAYSYSTEM_H
#define COLORDISPLAYSYSTEM_H

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>

class ColorDisplaySystem : public ecs::System
{
private:
    Shader *m_shader;
    std::vector<float> m_vectVertex;
    VerticesData m_verticesData;
private:
    void fillVertexFromEntities();
    void drawVertex();
public:
    ColorDisplaySystem();
    void setUsedComponents();
    void execSystem()override;
    void setShader(Shader &shader);
    void display()const;
};

#endif // COLORDISPLAYSYSTEM_H
