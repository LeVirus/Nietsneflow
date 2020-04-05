#pragma once

#include <BaseECS/system.hpp>
#include <functional>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>

struct MapCoordComponent;

struct PlayerComp
{
    MapCoordComponent const *m_mapCoordComp = nullptr;
    PositionVertexComponent const *m_posComp = nullptr;
    ColorVertexComponent const *m_colorComp = nullptr;
};

class MapDisplaySystem : public ecs::System
{
private:
    Shader *m_shader;
    std::vector<VerticesData> m_vectVerticesData;
    uint32_t m_playerNum;
    float m_levelSizePX, m_localLevelSizePX;
    float m_tileSizeGL;
    std::vector<uint32_t> m_entitiesToDisplay;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    PlayerComp m_playerComp;
private:
    void setUsedComponents();
    void fillVertexFromEntities();
    void drawVertex();
    void drawPlayerOnMap();
    void confPositionVertexEntities();
    void confVertexElement(const pairFloat_t &glPosition, uint32_t entityNum);
    void setVertexStaticElementPosition(uint32_t entityNum);
    bool checkBoundEntityMap(const MapCoordComponent &mapCoordComp, const pairUI_t &minBound, const pairUI_t &maxBound);
    void getMapDisplayLimit(pairFloat_t &playerPos, pairUI_t &min, pairUI_t &max);
    pairFloat_t getUpLeftCorner(const MapCoordComponent *mapCoordComp, uint32_t entityNum);
public:
    MapDisplaySystem();
    void confLevelData();
    void confPlayerComp(uint32_t playerNum);
    void setVectTextures(std::vector<Texture> &vectTexture);
    void execSystem()override;
    void setShader(Shader &shader);
};

//Adapt to GL context
template <typename T>
std::pair<T,T> operator-(const std::pair<T,T> & l,const std::pair<T,T> & r) {
    return {l.first - r.first,r.second - l.second};
}
