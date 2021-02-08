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
    Texture_e m_textureNum;
    uint32_t m_entityNum;
    EntityData(float distance, Texture_e textureNum, uint32_t entityNum) : m_distance(distance),
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
    void treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA,
                            MapCoordComponent *mapCompB,
                            VisionComponent *visionComp, uint32_t &toRemove,
                            float observerAngle, uint32_t numIteration);
    void fillWallEntitiesData(uint32_t numEntity, pairFloat_t absolPos[], VisionComponent *visionComp,
                              MapCoordComponent *mapCompCamera,
                              MapCoordComponent *mapCompB, float radiantObserverAngle, bool pointIn[],
                              bool outLeft[], uint32_t &angleToTreat);
    //Fill data in sort of displaying wall from left to right
    void fillAbsolAndDistanceWall(pairFloat_t absolPos[], float distance[],
                                  VisionComponent *visionComp, MapCoordComponent *mapCompA,
                                  MapCoordComponent *mapCompB, uint32_t numEntity,
                                  uint32_t &distanceToTreat, float degreeObserverAngle);
    void confNormalEntityVertex(uint32_t numEntity,
                                VisionComponent *visionComp, float lateralPosDegree, float depthGL);
    void confWallEntityVertex(uint32_t numEntity, VisionComponent *visionComp,
                              float lateralPosDegree[], float distance[], bool wallAllVisible);
    void drawVertex();
    pairFloat_t getCenterPosition(MapCoordComponent const *mapComp,
                                  GeneralCollisionComponent *genCollComp, float numEntity);
    void fillVertexFromEntity(uint32_t numEntity, uint32_t numIteration, float distance, bool wallTag = false);
    std::optional<pairFloat_t> checkLimitWallCase(const pairFloat_t &pointObserver, float limitObserverAngle,
                                                  const pairFloat_t &outPoint, const pairFloat_t &linkPoint,
                                                  bool leftLimit, bool XCase, float correction,
                                                  pairFloat_t &pointReturn);
    //check if a wall rect is visible in terms of others wall
    bool angleWallVisible(const pairFloat_t &observerPoint, const pairFloat_t &angleWall,
                          const std::vector<uint32_t> &vectEntities, uint32_t numEntity);
    void calculateDepthWallEntitiesData(uint32_t numEntity, uint32_t angleToTreat,
                                        const bool pointIn[], const bool outLeft[], float depthGL[],
                                        float radiantObserverAngle, const pairFloat_t absolPos[],
                                        const MapCoordComponent *mapCompCamera);
private:
    Shader *m_shader;
    std::multiset<EntityData> m_entitiesNumMem;
    std::vector<VerticesData> m_vectVerticesData;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    //number of entity to draw per player
    vectUI_t m_numVertexToDraw;
    uint32_t m_textureLineDrawNumber = 100;
};

//FOR 90 degree ONLY !!!!!!!!
float getQuarterAngle(float angle);
uint32_t getMaxValueFromEntries(const float distance[4]);
uint32_t getMinValueFromEntries(const float distance[4]);
std::optional<uint32_t> getLimitIndex(const bool pointIn[], const float distanceReal[], uint32_t i);
float getFPSLateralGLPosFromAngle(float centerAngleVision, const pairFloat_t &observerPoint,
                                  const pairFloat_t &targetPoint);
pairFloat_t getPairFPSLateralGLPosFromAngle(float centerAngleVision, const pairFloat_t &observerPoint,
                                            const pairFloat_t &targetPointA, const pairFloat_t &targetPointB,
                                            bool pointIn[], bool outLeft[], std::optional<float> firstResult);
float getLateralAngle(float centerAngleVision, float trigoAngle);
float getIntersectCoord(const pairFloat_t &observerPoint, const pairFloat_t &targetPointA, float centerAngleVision, bool outLeft, bool YIntersect);
float calculateDepthGLAngleWallLimitDisplay(const pairFloat_t &outPoint, const pairFloat_t &linkPoint,
                                            float observerAngle, MapCoordComponent *mapCompCamera, SpriteTextureComponent *spriteComp);
void treatLimitAngle(float &degreeAngleA, float &degreeAngleB);
void removeSecondRect(pairFloat_t absolPos[], float distance[], uint32_t &distanceToTreat);
