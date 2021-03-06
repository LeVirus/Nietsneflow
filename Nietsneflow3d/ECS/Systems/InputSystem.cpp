#include "InputSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include "PhysicalEngine.hpp"
#include <cassert>


//===================================================================
InputSystem::InputSystem()
{
    setUsedComponents();
}

//===================================================================
void InputSystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::INPUT_COMPONENT);
}

//===================================================================
void InputSystem::treatPlayerInput()
{
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, true);
        return;
    }
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i],
                                                         Components_e::MAP_COORD_COMPONENT);
        assert(mapComp);
        MoveableComponent *moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(mVectNumEntity[i],
                                                         Components_e::MOVEABLE_COMPONENT);
        assert(moveComp);
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(mVectNumEntity[i],
                                                         Components_e::POSITION_VERTEX_COMPONENT);
        assert(posComp);
        VisionComponent *visionComp = stairwayToComponentManager().
                searchComponentByType<VisionComponent>(mVectNumEntity[i],
                                                         Components_e::VISION_COMPONENT);
        assert(visionComp);
        //STRAFE
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        {
            moveElement(*moveComp, *mapComp, MoveOrientation_e::RIGHT);
        }
        else if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            moveElement(*moveComp, *mapComp, MoveOrientation_e::LEFT);
        }
        if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            moveElement(*moveComp, *mapComp, MoveOrientation_e::FORWARD);
        }
        else if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            moveElement(*moveComp, *mapComp, MoveOrientation_e::BACKWARD);
        }
        if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            moveComp->m_degreeOrientation -= moveComp->m_rotationAngle;
            if(moveComp->m_degreeOrientation < 0.0f)
            {
                moveComp->m_degreeOrientation += 360.0f;
            }
            updatePlayerOrientation(*moveComp, *posComp, *visionComp);
        }
        else if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            moveComp->m_degreeOrientation += moveComp->m_rotationAngle;
            if(moveComp->m_degreeOrientation > 360.0f)
            {
                moveComp->m_degreeOrientation -= 360.0f;
            }
            updatePlayerOrientation(*moveComp, *posComp, *visionComp);
        }
    }
}

//===================================================================
void InputSystem::execSystem()
{
    System::execSystem();
    treatPlayerInput();
}

