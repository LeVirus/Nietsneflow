#pragma once

#include <vector>
#include <set>
#include <BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <PhysicalEngine.hpp>

struct GeneralCollisionComponent;
struct RectangleCollisionComponent;
struct RayCastingIntersect;
struct ElementRaycast;

using vectUI_t = std::vector<uint32_t>;
using mapRayCastingData_t = std::map<uint32_t, std::vector<RayCastingIntersect>>;
using pairRaycastingData_t = std::pair<uint32_t, std::vector<RayCastingIntersect>>;
using tupleTargetRaycast_t = std::tuple<pairFloat_t, float, std::optional<uint32_t>>;
using optionalTargetRaycast_t = std::optional<tupleTargetRaycast_t>;
using vectPairFloatPairFloat_t = std::vector<std::pair<float, pairFloat_t>>;

struct GroundCeililngRayCastingIntersect
{
    //first distance second texture pos
    std::vector<std::pair<float, pairFloat_t>> m_vectIntersect;
    uint32_t m_lateral;
};

struct RayCastingIntersect
{
    float m_distance, m_texturePos;
    uint32_t m_lateral;
};

struct EntityData
{
    float m_distance;
    uint32_t m_textureNum;
    uint32_t m_iterationNum;
    EntityData(float distance, uint32_t textureNum, uint32_t iterationNum) : m_distance(distance),
        m_textureNum(textureNum), m_iterationNum(iterationNum)
    {}

    bool operator<(const EntityData& rhs)const
    {
        return m_distance > rhs.m_distance;
    }
};

const uint32_t RAYCAST_LINE_NUMBER = 200;

class FirstPersonDisplaySystem : public ecs::System
{
public:
    FirstPersonDisplaySystem();
    void execSystem()override;
    void drawPlayerDamage();
    void setVectTextures(std::vector<Texture> &vectTexture);
    void setShader(Shader &shader);
    inline void memGroundBackgroundEntity(uint32_t entity)
    {
        m_groundEntity = entity;
    }
    inline void memCeilingBackgroundEntity(uint32_t entity)
    {
        m_ceilingEntity = entity;
    }
    //return target point, texture position and entity num if collision
    optionalTargetRaycast_t calcLineSegmentRaycast(float radiantAngle,
                                                   const pairFloat_t &originPoint,
                                                   bool visual = true, uint32_t iteration = 0);
private:
    optionalTargetRaycast_t calcDoorSegmentRaycast(float radiantAngle,
                                                   std::optional<float> lateralLeadCoef,
                                                   std::optional<float> verticalLeadCoef,
                                                   pairFloat_t &currentPoint,
                                                   const ElementRaycast &element);
    bool behindRaycastElement(const MapCoordComponent *mapCompObserver, const MapCoordComponent *mapCompTarget, float distance,
                              float radiantObserverAngle, uint32_t targetEntity);
    void rayCasting();
    std::optional<float> treatDoorRaycast(uint32_t numEntity, float currentRadiantAngle,
                          pairFloat_t &currentPoint, std::optional<float> lateralLeadCoef,
                          std::optional<float> verticalLeadCoef,
                          bool &textLateral, bool &textFace);
    void memDistance(uint32_t numEntity, uint32_t lateralScreenPos, float distance, float texturePos);
    void setUsedComponents();
    void confCompVertexMemEntities();
    void writeVertexWallDoorRaycasting(const pairRaycastingData_t &entityData, uint32_t numIteration);
    void treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA,
                            MapCoordComponent *mapCompB, VisionComponent *visionComp,
                            uint32_t &toRemove, float degreeObserverAngle, uint32_t numIteration);
    void confNormalEntityVertex(uint32_t numEntity,
                                VisionComponent *visionComp, CollisionTag_e tag, float lateralPosGL, float distance);
    void drawVertex();
    void drawTextureBackground();
    pairFloat_t getCenterPosition(MapCoordComponent const *mapComp,
                                  GeneralCollisionComponent *genCollComp, float numEntity);
    void fillVertexFromEntity(uint32_t numEntity, uint32_t numIteration, float distance,
                              DisplayMode_e displayMode);
    VerticesData &getClearedVertice(uint32_t index);
    void writeVertexGroundCeiling();
    inline void clearGroundCeilingRaycast()
    {
        for(uint32_t i = 0; i < m_groundCeilingRaycastPoint.size(); ++i)
        {
            m_groundCeilingRaycastPoint[i].clear();
        }
    }
private:
    Shader *m_shader;
    std::multiset<EntityData> m_entitiesNumMem;
    std::vector<VerticesData> m_vectWallDoorVerticesData;
    VerticesData m_groundVertice, m_ceilingVertice;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    mapRayCastingData_t m_raycastingData;
    //number of entity to draw per player
    vectUI_t m_numVertexToDraw;
    std::array<std::vector<pairFloat_t>, RAYCAST_LINE_NUMBER> m_groundCeilingRaycastPoint;
    float m_stepAngle = getRadiantAngle(CONE_VISION / static_cast<float>(RAYCAST_LINE_NUMBER)),
    m_stepDrawLateralScreen = 2.0f / static_cast<float>(RAYCAST_LINE_NUMBER);
    std::optional<uint32_t> m_groundEntity, m_ceilingEntity;
};

float getQuarterAngle(float angle);
uint32_t getMaxValueFromEntries(const float distance[4]);
uint32_t getMinValueFromEntries(const float distance[4]);
std::optional<uint32_t> getLimitIndex(const bool pointIn[], const float distanceReal[], uint32_t i);
float getLateralAngle(float centerAngleVision, float trigoAngle);
pairFloat_t getIntersectCoord(const pairFloat_t &observerPoint, const pairFloat_t &targetPoint,
                              float centerAngleVision, bool outLeft, bool YIntersect);
bool treatDisplayDoor(float currentRadiantAngle, bool doorVertical, pairFloat_t &currentPoint,
                      const pairFloat_t doorPos[], std::optional<float> verticalLeadCoef,
                      std::optional<float> lateralLeadCoef, bool &textLateral, bool &textFace, bool bull = false);
bool treatVerticalIntersectDoor(pairFloat_t &currentPoint, const pairFloat_t doorPos[],
                               float verticalLeadCoef, float radiantAngle);
//return true if door collision
bool treatLateralIntersectDoor(pairFloat_t &currentPoint, const pairFloat_t doorPos[],
                                float lateralLeadCoef, float radiantAngle);
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
std::optional<pairUI_t> getCorrectedCoord(const pairFloat_t &currentPoint, bool lateral,
                                          float radiantAngle);
