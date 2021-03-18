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
struct RectangleCollisionComponent;
struct RayCastingIntersect;

using vectUI_t = std::vector<uint32_t>;
using mapRayCastingData_t = std::map<uint32_t, std::vector<RayCastingIntersect>>;
using pairRaycastingData_t = std::pair<uint32_t, std::vector<RayCastingIntersect>>;

struct RayCastingIntersect
{
    float m_distance, m_texturePos;
    uint32_t m_lateral;
};

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
    uint32_t m_iterationNum;
    EntityData(float distance, Texture_e textureNum, uint32_t iterationNum) : m_distance(distance),
        m_textureNum(textureNum), m_iterationNum(iterationNum)
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
    void rayCasting();
    float getTexturePos(uint32_t numEntity, const pairFloat_t &position, bool lateral);
    bool treatDoorRaycast(uint32_t numEntity, uint32_t radiantAngle, pairFloat_t &currentPoint, const pairUI_t &coord, bool lateral, std::optional<float> lateralLeadCoef, std::optional<float> verticalLeadCoef);
    void memDistance(uint32_t numEntity, uint32_t lateralScreenPos, float distance, float texturePos);
    void setUsedComponents();
    void confCompVertexMemEntities();
    void writeVertexRaycasting(const pairRaycastingData_t &entityData, uint32_t numIteration);
    void adaptTextureDoorDisplay(DoorComponent *doorComp,
                                 RectangleCollisionComponent *rectComp, MapCoordComponent *mapCompCamera,
                                 MapCoordComponent *mapCompDoor, pairFloat_t absolPos[]);
    void treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA,
                            MapCoordComponent *mapCompB,
                            VisionComponent *visionComp, uint32_t &toRemove,
                            float degreeObserverAngle, uint32_t numIteration);
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
    void fillVertexFromEntity(uint32_t numEntity, uint32_t numIteration, float distance,
                              DisplayMode_e displayMode);
    VerticesData &getClearedVertice(uint32_t index);

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
    mapRayCastingData_t m_raycastingData;
    //number of entity to draw per player
    vectUI_t m_numVertexToDraw;
    uint32_t m_textureLineDrawNumber = 200;
    float m_stepAngle = CONE_VISION / static_cast<float>(m_textureLineDrawNumber),
    m_stepDrawLateralScreen = 2.0f / static_cast<float>(m_textureLineDrawNumber);
};

//FOR 90 degree ONLY !!!!!!!!
float getQuarterAngle(float angle);
uint32_t getMaxValueFromEntries(const float distance[4]);
uint32_t getMinValueFromEntries(const float distance[4]);
std::optional<uint32_t> getLimitIndex(const bool pointIn[], const float distanceReal[], uint32_t i);
float getLateralAngle(float centerAngleVision, float trigoAngle);
pairFloat_t getIntersectCoord(const pairFloat_t &observerPoint, const pairFloat_t &targetPoint,
                              float centerAngleVision, bool outLeft, bool YIntersect);
void treatLimitAngle(float &degreeAngleA, float &degreeAngleB);
void removeSecondRect(pairFloat_t absolPos[], float distance[], uint32_t &distanceToTreat);
float getDoorDistance(const MapCoordComponent *mapCompCamera, const MapCoordComponent *mapCompDoor,
                      const DoorComponent *doorComp);
float getMiddleDoorDistance(const pairFloat_t &camera, const pairFloat_t &element, bool vertical);
std::optional<float> getModulo(float sinCosAngle, float position, float modulo, bool lateral);
//lateral == false vertical
std::optional<float> getLeadCoef(float radiantAngle, bool lateral);
pairFloat_t getLimitPointRayCasting(const pairFloat_t &cameraPoint, float radiantAngle, std::optional<float> lateralLeadCoef, std::optional<float> verticalLeadCoef, bool &lateral);
int32_t getCoord(float value, float tileSize);
bool treatLimitIntersect(pairFloat_t &point, bool lateral);
