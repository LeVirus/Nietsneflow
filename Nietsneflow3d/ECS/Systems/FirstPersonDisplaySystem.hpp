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
using MapRayCastingData_t = std::map<uint32_t, std::vector<RayCastingIntersect>>;
using pairRaycastingData_t = std::pair<uint32_t, std::vector<RayCastingIntersect>>;
using tupleTargetRaycast_t = std::tuple<PairFloat_t, float, std::optional<uint32_t>>;
using optionalTargetRaycast_t = std::optional<tupleTargetRaycast_t>;
using vectPairFloatPairFloat_t = std::vector<std::pair<float, PairFloat_t>>;

float randFloat(float min, float max);

struct GroundCeililngRayCastingIntersect
{
    //first distance second texture pos
    std::vector<std::pair<float, PairFloat_t>> m_vectIntersect;
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

    bool operator<(const EntityData &rhs)const
    {
        return m_distance > rhs.m_distance;
    }
};


class FirstPersonDisplaySystem : public ecs::System
{
public:
    FirstPersonDisplaySystem();
    void execSystem()override;
    void drawPlayerColorEffects();
    void setVectTextures(std::vector<Texture> &vectTexture);
    void addShaders(Shader &shaderTextColor, Shader &shaderText);
    void memGroundBackgroundEntity(uint32_t entity, bool simpleTexture);
    void memCeilingBackgroundEntity(uint32_t entity, bool simpleTexture);
    void clearBackgroundData();
    //return target point, texture position and entity num if collision
    optionalTargetRaycast_t calcLineSegmentRaycast(float radiantAngle, const PairFloat_t &originPoint, bool visual, bool scratchMode = false);
private:
    void setShader(Shader &shader);
    optionalTargetRaycast_t calcDoorSegmentRaycast(float radiantAngle, std::optional<float> lateralLeadCoef,
                                                   std::optional<float> verticalLeadCoef, PairFloat_t &currentPoint,
                                                   const ElementRaycast &element);
    optionalTargetRaycast_t calcMovingWallSegmentRaycast(float radiantAngle, std::optional<float> lateralLeadCoef,
                                                         std::optional<float> verticalLeadCoef, PairFloat_t &currentPoint,
                                                         const ElementRaycast &element);
    optionalTargetRaycast_t getTextureLimitCase(float radiantAngle, float lateralLeadCoef, float verticalLeadCoef,
                                                const PairUI_t &currentCoord,
                                                const PairFloat_t &currentPoint, bool lateral);
    std::optional<float> getCloserRaycastDistance(const MapCoordComponent *mapCompObserver, const MapCoordComponent *mapCompTarget, float distance,
                              float radiantObserverAngle, uint32_t targetEntity);
    bool rayCasting(uint32_t observerEntity);
    bool isInsideWall(const PairFloat_t &pos);
    void calcVerticalBackgroundLineRaycast(const PairFloat_t &observerPos, float currentRadiantAngle,
                                              float currentGLLatPos, float radiantObserverAngle);
    std::optional<float> treatDoorRaycast(uint32_t numEntity, float currentRadiantAngle,
                                          PairFloat_t &currentPoint, std::optional<float> lateralLeadCoef,
                                          std::optional<float> verticalLeadCoef, bool &textLateral, bool &textFace);
    void memRaycastDistance(uint32_t numEntity, uint32_t lateralScreenPos, float distance, float texturePos);
    void setUsedComponents();
    void confCompVertexMemEntities();
    void writeVertexWallDoorRaycasting(const pairRaycastingData_t &entityData, uint32_t numIteration);
    void treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA,
                            MapCoordComponent *mapCompB, VisionComponent *visionComp,
                            uint32_t &toRemove, float degreeObserverAngle, uint32_t numIteration, uint32_t currentNormal);
    bool elementBehindDoor(const ElementRaycast &elementCase, float radiantObserverAngle, const MapCoordComponent *mapComp);
    bool confNormalEntityVertex(const std::pair<uint32_t, bool> &entityData, VisionComponent *visionComp, CollisionTag_e tag, float lateralPosGL, float distance);
    void drawVertex();
    void drawTextureBackground();
    PairFloat_t getCenterPosition(MapCoordComponent const *mapComp, GeneralCollisionComponent *genCollComp, float numEntity);
    void fillVertexFromEntity(uint32_t numEntity, uint32_t numIteration, float distance);
    VerticesData &getClearedVertice(uint32_t index);
    void confSimpleTextVertexGroundCeiling(float observerAngle);
    void writeSimpleTextVertexGroundCeiling();
private:
    Shader *m_shader;
    std::map<uint32_t, uint32_t> m_memDoorDistance;
    std::map<uint32_t, float> m_memWallEntityDistances;
    std::multiset<EntityData> m_entitiesNumMem;
    std::vector<VerticesData> m_vectWallDoorVerticesData;
    VerticesData m_groundSimpleTextVertice, m_groundTiledTextVertice, m_ceilingSimpleVertice, m_ceilingTiledVertice;
    std::optional<PairFloat_t> m_ceilingTextureSize, m_groundTextureSize;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    MapRayCastingData_t m_raycastingData;
    //number of entity to draw per player
    vectUI_t m_numVertexToDraw;
    float m_stepAngle = getRadiantAngle(CONE_VISION / static_cast<float>(RAYCAST_LINE_NUMBER));
    //second :: false = tiled texture     true = simple texture
    std::optional<uint32_t> m_groundSimpleTextBackground, m_groundTiledTextBackground,
    m_ceilingSimpleTextBackground, m_ceilingTiledTextBackground;
    bool m_backgroundRaycastActive = false, m_groundCeilingSimpleTextureActive = false;
    std::array<float, RAYCAST_LINE_NUMBER> m_memRaycastDist;
    std::optional<std::array<float, RAYCAST_GROUND_CEILING_NUMBER>> m_memBackgroundDistance;
    //first coloredTexture, second texture
    std::pair<Shader*, Shader*> m_memShaders;
};

float getFogIntensity(float distance);
float getRaycastTexturePos(float radiantObserverAngle, bool lateral, const PairFloat_t &pos);
float getDoorRaycastTexturePos(float textDoor, float radiantObserverAngle, bool lateral, const PairFloat_t &pos);
//if player pos is on limit case modify position to prevent display issue
PairFloat_t getCorrectedPosition(const PairFloat_t &initPos, float radiantAngle);
float getQuarterAngle(float angle);
uint32_t getMaxValueFromEntries(const float distance[4]);
uint32_t getMinValueFromEntries(const float distance[4]);
std::optional<uint32_t> getLimitIndex(const bool pointIn[], const float distanceReal[], uint32_t i);
float getLateralAngle(float centerAngleVision, float trigoAngle);
PairFloat_t getIntersectCoord(const PairFloat_t &observerPoint, const PairFloat_t &targetPoint,
                              float centerAngleVision, bool outLeft, bool YIntersect);
bool treatDisplayDoor(float currentRadiantAngle, bool doorVertical, PairFloat_t &currentPoint,
                      const PairFloat_t doorPos[], std::optional<float> verticalLeadCoef,
                      std::optional<float> lateralLeadCoef, bool &textLateral, bool &textFace, bool bull = false);
bool treatVerticalIntersectRect(PairFloat_t &currentPoint, const PairFloat_t rectPos[], float verticalLeadCoef, float radiantAngle);
//return true if door collision
bool treatLateralIntersectRect(PairFloat_t &currentPoint, const PairFloat_t rectPos[], float lateralLeadCoef, float radiantAngle);
void treatLimitAngle(float &degreeAngleA, float &degreeAngleB);
void removeSecondRect(PairFloat_t absolPos[], float distance[], uint32_t &distanceToTreat);
float getDoorDistance(const MapCoordComponent *mapCompCamera, const MapCoordComponent *mapCompDoor, const DoorComponent *doorComp);
float getMiddleDoorDistance(const PairFloat_t &camera, const PairFloat_t &element, bool vertical);
std::optional<float> getModulo(float sinCosAngle, float position, float modulo, bool lateral);
//lateral == false vertical
std::optional<float> getLeadCoef(float radiantAngle, bool lateral);
bool raycastPointLateral(float radiantAngle, const PairFloat_t &cameraPoint);
PairFloat_t getLimitPointRayCasting(const PairFloat_t &cameraPoint, float radiantAngle, std::optional<float> lateralLeadCoef, std::optional<float> verticalLeadCoef, bool &lateral);
int32_t getCoord(float value, float tileSize);
std::optional<PairUI_t> getCorrectedCoord(const PairFloat_t &currentPoint, bool lateral, float radiantAngle);
