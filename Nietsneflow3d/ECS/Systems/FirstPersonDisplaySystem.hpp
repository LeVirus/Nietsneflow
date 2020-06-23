#pragma once

#include <vector>
#include <set>
#include <BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>

struct GeneralCollisionComponent;

using vectUI_t = std::vector<uint32_t>;

enum WallTreatment_e
{
    NORMAL,
    REMOVE_ZERO,
    REMOVE_TWO
};

struct EntityData
{
    float m_distance;
    Texture_t m_textureNum;
    uint32_t m_entityNum;
    EntityData(float distance, Texture_t textureNum, uint32_t entityNum) : m_distance(distance),
        m_textureNum(textureNum), m_entityNum(entityNum)
    {}

    bool operator<(const EntityData& rhs)const
    {
        return m_distance > rhs.m_distance;
    }
};

class FirstPersonDisplaySystem : public ecs::System
{
public:
    FirstPersonDisplaySystem();
    void execSystem()override;
    void setVectTextures(std::vector<Texture> &vectTexture);
    void setShader(Shader &shader);
private:
    void setUsedComponents();
    void confCompVertexMemEntities();
    void treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA, MapCoordComponent *mapCompB,
                            VisionComponent *visionComp, uint32_t &toRemove,
                            float leftAngleVision, uint32_t numIteration);
    void fillWallEntitiesData(uint32_t numEntity, pairFloat_t absolPos[], float distance[], MapCoordComponent *mapCompA,
                              MapCoordComponent *mapCompB, float observerAngle, VisionComponent *visionComp, bool pointIn[], bool outLeft[]);
    void fillAbsolAndDistanceWall(pairFloat_t absolPos[], float distance[], MapCoordComponent *mapCompA, MapCoordComponent *mapCompB, uint32_t numEntity);
    void confNormalEntityVertex(uint32_t numEntity,
                                VisionComponent *visionComp, float lateralPosDegree, float distance);
    void confWallEntityVertex(uint32_t numEntity, VisionComponent *visionComp,
                              float lateralPosDegree[], float distance[]);
    void drawVertex();
    pairFloat_t getCenterPosition(MapCoordComponent const *mapComp, GeneralCollisionComponent *genCollComp, float numEntity);
    void fillVertexFromEntitie(uint32_t numEntity, uint32_t numIteration, float distance);
    void setPointCameraLimitWall(const pairFloat_t &pointObserver, float observerAngle,
                                             pairFloat_t &outPoint, const pairFloat_t &linkPoint,
                                             bool leftLimit, VisionComponent *visionComp);
private:
    Shader *m_shader;
    std::set<EntityData> m_entitiesNumMem;
    std::vector<VerticesData> m_vectVerticesData;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    //number of entity to draw per player
    vectUI_t m_numVertexToDraw;
};

float getQuarterAngle(float angle, bool leftSense);
uint32_t getMaxValueFromEntries(const float distance[4]);
uint32_t getMinValueFromEntries(const float distance[4]);
