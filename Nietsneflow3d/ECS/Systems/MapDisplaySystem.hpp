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
    uint32_t m_playerNum;
    Shader *m_shader;
    VerticesData m_verticesData;
private:
    void setUsedComponents();
    void fillVertexFromEntities();
    void drawVertex();
    void drawPlayerOnMap();
    void confEntity();
public:
    MapDisplaySystem();

    inline void setPlayerEntityNum(uint32_t playerNum)
    {
        m_playerNum = playerNum;
    }

    void execSystem()override;
    void setShader(Shader &shader);
};

#endif // MAPDISPLAYSYSTEM_H
