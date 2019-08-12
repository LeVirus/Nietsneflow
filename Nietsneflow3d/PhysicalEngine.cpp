#include "PhysicalEngine.hpp"
#include <math.h>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>


//===================================================================
PhysicalEngine::PhysicalEngine()
{

}

//===================================================================
void PhysicalEngine::runIteration()
{
    m_inputSystem->execSystem();
}

//===================================================================
void PhysicalEngine::linkSystems(InputSystem *inputSystem)
{
    m_inputSystem = inputSystem;
}

//===================================================================
void updatePlayerOrientation(const MoveableComponent &moveComp,
                              PositionVertexComponent &posComp)
{
    if(posComp.m_vertex.empty())
    {
        posComp.m_vertex.resize(3);
    }
    int32_t angle = moveComp.m_degreeOrientation;
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

float getRadiantAngle(int32_t angle)
{
    return angle * PI / 180;
}
