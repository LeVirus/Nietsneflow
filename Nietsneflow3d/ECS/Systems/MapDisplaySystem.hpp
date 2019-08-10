#ifndef MAPDISPLAYSYSTEM_H
#define MAPDISPLAYSYSTEM_H

#include <BaseECS/system.hpp>
#include <functional>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>

using pairUI_t = std::pair<uint32_t, uint32_t>;
using pairFloat_t = std::pair<float, float>;

struct MapCoordComponent;

class MapDisplaySystem : public ecs::System
{
private:
    uint32_t m_playerNum;
    Shader *m_shader;
    VerticesData m_verticesData;
    std::vector<uint32_t> m_entitiesToDisplay;
    float m_levelSizePX, m_localLevelSizePX;
    float m_tileSizeGL;
private:
    void setUsedComponents();
    void fillVertexFromEntities();
    void drawVertex();
    void drawPlayerOnMap();
    void confVertexEntities();
    void confVertexElement(const pairFloat_t &glPosition, uint32_t entityNum);
    void setVertexStaticElementPosition(uint32_t entityNum);
    bool checkBoundEntityMap(const MapCoordComponent &mapCoordComp, const pairUI_t &minBound, const pairUI_t &maxBound);
    void getMapDisplayLimit(pairFloat_t &playerPos, pairUI_t &min, pairUI_t &max);
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

//Adapt to GL context
template <typename T>
std::pair<T,T> operator-(const std::pair<T,T> & l,const std::pair<T,T> & r) {
    return {l.first - r.first,r.second - l.second};
}
#endif // MAPDISPLAYSYSTEM_H
