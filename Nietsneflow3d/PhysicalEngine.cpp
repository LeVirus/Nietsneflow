#include "PhysicalEngine.hpp"
#include "Level.hpp"
#include <math.h>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/WeaponComponent.hpp>
#include <ECS/Systems/DoorSystem.hpp>
#include <ECS/Systems/IASystem.hpp>
#include <cassert>

//===================================================================
PhysicalEngine::PhysicalEngine()
{

}

//===================================================================
void PhysicalEngine::runIteration(bool gamePaused)
{
    m_inputSystem->execSystem();
    if(!gamePaused)
    {
        m_doorSystem->execSystem();
        m_collisionSystem->execSystem();
        m_iaSystem->execSystem();
    }
}

//===================================================================
void PhysicalEngine::linkSystems(InputSystem *inputSystem, CollisionSystem *collisionSystem,
                                 DoorSystem *doorSystem, IASystem *iaSystem)
{
    m_inputSystem = inputSystem;
    m_collisionSystem = collisionSystem;
    m_doorSystem = doorSystem;
    m_iaSystem = iaSystem;
}

//===================================================================
void PhysicalEngine::memPlayerEntity(uint32_t playerEntity)
{
    assert(m_iaSystem);
    m_iaSystem->memPlayerDatas(playerEntity);
    m_collisionSystem->memPlayerDatas(playerEntity);
}

//===================================================================
void PhysicalEngine::confPlayerVisibleShoot(std::vector<uint32_t> &visibleShots,
                                            const pairFloat_t &point, float degreeAngle)
{
    m_iaSystem->confVisibleShoot(visibleShots, point, degreeAngle, CollisionTag_e::BULLET_PLAYER_CT);
}

//===================================================================
void PhysicalEngine::setModeTransitionMenu(bool transition)
{
    m_inputSystem->setModeTransitionMenu(transition);
}

//===================================================================
void moveElementFromAngle(float distanceMove, float radiantAngle,
                          pairFloat_t &point, bool playerMove)
{
    point.first += std::cos(radiantAngle) * distanceMove;
    //limit case
    if(playerMove && point.first < distanceMove)
    {
        point.first = distanceMove;
    }
    point.second -= std::sin(radiantAngle) * distanceMove;
    //limit case
    if(playerMove && point.second < distanceMove)
    {
        point.second = distanceMove;
    }
}

//===================================================================
void updatePlayerOrientation(const MoveableComponent &moveComp,
                             PositionVertexComponent &posComp, VisionComponent &visionComp)
{
    updatePlayerArrow(moveComp, posComp);
    updatePlayerConeVision(moveComp, visionComp);
}

//===================================================================
void updatePlayerArrow(const MoveableComponent &moveComp, PositionVertexComponent &posComp)
{
    if(posComp.m_vertex.empty())
    {
        posComp.m_vertex.resize(3);
    }
    float angle = moveComp.m_degreeOrientation;
    float radiantAngle = getRadiantAngle(angle);
    posComp.m_vertex[0].first = MAP_LOCAL_CENTER_X_GL +
            cos(radiantAngle) * PLAYER_RAY_DISPLAY;
    posComp.m_vertex[0].second = MAP_LOCAL_CENTER_Y_GL +
            sin(radiantAngle) * PLAYER_RAY_DISPLAY;
    angle += 150.0f;
    radiantAngle = getRadiantAngle(angle);

    posComp.m_vertex[1].first = MAP_LOCAL_CENTER_X_GL +
            cos(radiantAngle) * PLAYER_RAY_DISPLAY;
    posComp.m_vertex[1].second = MAP_LOCAL_CENTER_Y_GL +
            sin(radiantAngle) * PLAYER_RAY_DISPLAY;
    angle += 60.0f;
    radiantAngle = getRadiantAngle(angle);

    posComp.m_vertex[2].first = MAP_LOCAL_CENTER_X_GL +
            cos(radiantAngle) * PLAYER_RAY_DISPLAY;
    posComp.m_vertex[2].second = MAP_LOCAL_CENTER_Y_GL +
            sin(radiantAngle) * PLAYER_RAY_DISPLAY;
}

//===================================================================
void updatePlayerConeVision(const MoveableComponent &moveComp, VisionComponent &visionComp)
{

    float degreeAngle = moveComp.m_degreeOrientation - HALF_CONE_VISION;
    float radiantAngle = getRadiantAngle(degreeAngle);
    if(visionComp.m_positionVertexComp.m_vertex.empty())
    {
        visionComp.m_positionVertexComp.m_vertex.resize(3);
    }
    visionComp.m_positionVertexComp.m_vertex[0].first = MAP_LOCAL_CENTER_X_GL;
    visionComp.m_positionVertexComp.m_vertex[0].second = MAP_LOCAL_CENTER_Y_GL;
    visionComp.m_positionVertexComp.m_vertex[1].first = MAP_LOCAL_CENTER_X_GL +
            cos(radiantAngle) * PLAYER_RAY_DISPLAY * 5;//TEST
    visionComp.m_positionVertexComp.m_vertex[1].second = MAP_LOCAL_CENTER_Y_GL +
            sin(radiantAngle) * PLAYER_RAY_DISPLAY * 5;
    radiantAngle = getRadiantAngle(degreeAngle + CONE_VISION);
    visionComp.m_positionVertexComp.m_vertex[2].first = MAP_LOCAL_CENTER_X_GL +
            cos(radiantAngle) * PLAYER_RAY_DISPLAY * 5;//TEST
    visionComp.m_positionVertexComp.m_vertex[2].second = MAP_LOCAL_CENTER_Y_GL +
            sin(radiantAngle) * PLAYER_RAY_DISPLAY * 5;
}

//===================================================================
float getRadiantAngle(float angle)
{
    return angle * PI / 180;
}

//===================================================================
float getDegreeAngle(float angle)
{
    return angle / PI * 180;
}
