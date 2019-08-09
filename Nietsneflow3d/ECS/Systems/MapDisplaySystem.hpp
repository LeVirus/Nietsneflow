#ifndef MAPDISPLAYSYSTEM_H
#define MAPDISPLAYSYSTEM_H

#include <BaseECS/system.hpp>
#include <functional>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>

using pairUI_t = std::pair<uint32_t, uint32_t>;

struct MapCoordComponent;

class MapDisplaySystem : public ecs::System
{
private:
    uint32_t m_playerNum;
    Shader *m_shader;
    VerticesData m_verticesData;
    std::vector<uint32_t> m_entitiesToDisplay;
    float m_levelSizePX;
    float m_halfTileSizeGL;
private:
    void setUsedComponents();
    void fillVertexFromEntities();
    void drawVertex();
    void drawPlayerOnMap();
    void confEntity();
    void setVertexStaticElementPosition(uint32_t entityNum);
    bool checkBoundEntityMap(const MapCoordComponent &mapCoordComp, const pairUI_t &minBound, const pairUI_t &maxBound);
public:
    MapDisplaySystem();
    void confLevelData();
    inline void setPlayerEntityNum(uint32_t playerNum)
    {
        m_playerNum = playerNum;
    }

    void execSystem()override;
    void setShader(Shader &shader);
};

template <typename T>
std::pair<T,T> operator-(const std::pair<T,T> & l,const std::pair<T,T> & r) {
    return {l.first - r.first,l.second - r.second};
}
#endif // MAPDISPLAYSYSTEM_H
