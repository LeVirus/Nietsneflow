#include "PhysicalEngine.hpp"
#include "Level.hpp"
#include <math.h>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Systems/DoorSystem.hpp>


//===================================================================
PhysicalEngine::PhysicalEngine()
{

}

//===================================================================
void PhysicalEngine::runIteration()
{
    m_inputSystem->execSystem();
    m_doorSystem->execSystem();
    m_collisionSystem->execSystem();
}

//===================================================================
void PhysicalEngine::linkSystems(InputSystem *inputSystem, CollisionSystem *collisionSystem,
                                 DoorSystem *doorSystem)
{
    m_inputSystem = inputSystem;
    m_collisionSystem = collisionSystem;
    m_doorSystem = doorSystem;
}

//===================================================================
void moveElement(MoveableComponent &moveComp,
                 MapCoordComponent &mapComp, MoveOrientation_e moveDirection)
{
    float radiantAngle;
    float angle = moveComp.m_degreeOrientation;
    switch(moveDirection)
    {
    case MoveOrientation_e::FORWARD:
        break;
    case MoveOrientation_e::BACKWARD:
        angle += 180;
        break;
    case MoveOrientation_e::LEFT:
        angle += 90;
        break;
    case MoveOrientation_e::RIGHT:
        angle += 270;
        break;
    }
    moveComp.m_currentDegreeMoveDirection = angle;
    radiantAngle = getRadiantAngle(angle);
    mapComp.m_absoluteMapPositionPX.first +=
            cos(radiantAngle) * moveComp.m_velocity;
    if(mapComp.m_absoluteMapPositionPX.first < moveComp.m_velocity)
    {
        mapComp.m_absoluteMapPositionPX.first = moveComp.m_velocity;
    }
    mapComp.m_absoluteMapPositionPX.second -=
            sin(radiantAngle) * moveComp.m_velocity;
    if(mapComp.m_absoluteMapPositionPX.second < moveComp.m_velocity)
    {
        mapComp.m_absoluteMapPositionPX.second = moveComp.m_velocity;
    }
    mapComp.m_coord = *getLevelCoord(mapComp.m_absoluteMapPositionPX);
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
