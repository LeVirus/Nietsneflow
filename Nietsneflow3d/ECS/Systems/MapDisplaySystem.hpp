#ifndef MAPDISPLAYSYSTEM_H
#define MAPDISPLAYSYSTEM_H

#include <BaseECS/system.hpp>
#include <functional>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>

class MapDisplaySystem : public ecs::System
{
private:
    Shader *m_shader;
    VerticesData m_verticesData;
private:
    void setUsedComponents();
    void fillVertexFromEntities();
    void drawVertex();
public:
    MapDisplaySystem();
    void execSystem()override;
    void setShader(Shader &shader);
};

#endif // MAPDISPLAYSYSTEM_H
