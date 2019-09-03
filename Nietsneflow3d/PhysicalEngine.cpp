#include "PhysicalEngine.hpp"
#include <math.h>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>


//===================================================================
PhysicalEngine::PhysicalEngine()
{

}

//===================================================================
void PhysicalEngine::runIteration()
{
    m_inputSystem->execSystem();
    m_collisionSystem->execSystem();
}

//===================================================================
void PhysicalEngine::linkSystems(InputSystem *inputSystem,
                                 CollisionSystem *collisionSystem)
{
    m_inputSystem = inputSystem;
    m_collisionSystem = collisionSystem;
}

//===================================================================
void movePlayer(MoveableComponent &moveComp,
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
    moveComp.m_currentDegreeDirection = angle;
    radiantAngle = getRadiantAngle(angle);
    mapComp.m_absoluteMapPositionPX.first +=
            cos(radiantAngle) * moveComp.m_velocity;

    mapComp.m_absoluteMapPositionPX.second -=
            sin(radiantAngle) * moveComp.m_velocity;
}

//===================================================================
void updatePlayerOrientation(const MoveableComponent &moveComp,
                              PositionVertexComponent &posComp)
{
    if(posComp.m_vertex.empty())
    {
        posComp.m_vertex.resize(3);
    }
    int angle = static_cast<int>(moveComp.m_degreeOrientation);
    float radiantAngle = getRadiantAngle(angle);
    posComp.m_vertex[0].first = MAP_LOCAL_CENTER_X_GL +
            cos(radiantAngle) * PLAYER_RAY_DISPLAY;
    posComp.m_vertex[0].second = MAP_LOCAL_CENTER_Y_GL +
            sin(radiantAngle) * PLAYER_RAY_DISPLAY;
    angle += 150;
    angle %= 360;
    radiantAngle = getRadiantAngle(angle);

    posComp.m_vertex[1].first = MAP_LOCAL_CENTER_X_GL +
            cos(radiantAngle) * PLAYER_RAY_DISPLAY;
    posComp.m_vertex[1].second = MAP_LOCAL_CENTER_Y_GL +
            sin(radiantAngle) * PLAYER_RAY_DISPLAY;
    angle += 60;
    angle %= 360;
    radiantAngle = getRadiantAngle(angle);

    posComp.m_vertex[2].first = MAP_LOCAL_CENTER_X_GL +
            cos(radiantAngle) * PLAYER_RAY_DISPLAY;
    posComp.m_vertex[2].second = MAP_LOCAL_CENTER_Y_GL +
            sin(radiantAngle) * PLAYER_RAY_DISPLAY;
}

//===================================================================
float getRadiantAngle(float angle)
{
    return angle * PI / 180;
}
