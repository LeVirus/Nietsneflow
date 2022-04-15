#pragma once

#include <BaseECS/system.hpp>
#include <functional>
#include <set>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>

struct MapCoordComponent;
struct VisionComponent;

struct PlayerComp
{
    MapCoordComponent const *m_mapCoordComp = nullptr;
    PositionVertexComponent const *m_posComp = nullptr;
    ColorVertexComponent const *m_colorComp = nullptr;
    VisionComponent const *m_visionComp = nullptr;
};

class MapDisplaySystem : public ecs::System
{
public:
    MapDisplaySystem();
    void confLevelData();
    void confPlayerComp(uint32_t playerNum);
    void setVectTextures(std::vector<Texture> &vectTexture);
    void execSystem()override;
    void drawMiniMap();
    void drawFullMap();
    void setShader(Shader &shader);
    inline bool entityAlreadyDiscovered(uint32_t entityNum)const
    {
        return m_entitiesDetectedData.find(entityNum) != m_entitiesDetectedData.end();
    }
    inline void addDiscoveredEntity(uint32_t entityNum, const PairUI_t &pos)
    {
        m_entitiesDetectedData.insert({entityNum, pos});
    }
private:
    void confFullMapPositionVertexEntities();
    void drawPlayerOnFullMap();
    void setUsedComponents();
    void fillMapVertexFromEntities(bool miniMap);
    void drawMapVertex(bool miniMap);
    void drawPlayerVision();
    void drawPlayerOnMiniMap();
    void confMiniMapPositionVertexEntities();
    void confVertexElement(const PairFloat_t &glPosition, uint32_t entityNum);
    void setVertexStaticElementPosition(uint32_t entityNum);
    bool checkBoundEntityMap(const MapCoordComponent &mapCoordComp, const PairUI_t &minBound, const PairUI_t &maxBound);
    void getMapDisplayLimit(PairFloat_t &playerPos, PairUI_t &min, PairUI_t &max);
    PairFloat_t getUpLeftCorner(const MapCoordComponent *mapCoordComp, uint32_t entityNum);
private:
    std::map<uint32_t, PairUI_t> m_entitiesDetectedData;
    Shader *m_shader;
    std::vector<VerticesData> m_vectMiniMapVerticesData, m_vectFullMapVerticesData;
    float m_localLevelSizePX;
    float m_tileSizeGL;
    std::vector<uint32_t> m_entitiesToDisplay;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    PlayerComp m_playerComp;
};

//Adapt to GL context
template <typename T>
std::pair<T,T> operator-(const std::pair<T,T> & l,const std::pair<T,T> & r) {
    return {l.first - r.first,r.second - l.second};
}
