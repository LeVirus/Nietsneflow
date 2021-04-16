#include "InputSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/WriteComponent.hpp>
#include "PhysicalEngine.hpp"
#include <MainEngine.hpp>
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
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        if(m_mainEngine->isGamePaused())
        {
            treatMainMenu(mVectNumEntity[i]);
            continue;
        }
        if (!m_keyEspapePressed && glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(m_window, true);
            return;
        }
        else if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
        {
            m_keyEspapePressed = false;
        }
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
        PlayerConfComponent *playerComp = stairwayToComponentManager().
                searchComponentByType<PlayerConfComponent>(mVectNumEntity[i],
                                                         Components_e::PLAYER_CONF_COMPONENT);
        assert(playerComp);
        playerComp->m_inMovement = false;
        //STRAFE
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        {
            moveElement(*moveComp, *mapComp, MoveOrientation_e::RIGHT);
            playerComp->m_inMovement = true;
        }
        else if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            moveElement(*moveComp, *mapComp, MoveOrientation_e::LEFT);
            playerComp->m_inMovement = true;
        }
        if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            moveElement(*moveComp, *mapComp, MoveOrientation_e::FORWARD);
            playerComp->m_inMovement = true;
        }
        else if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            moveElement(*moveComp, *mapComp, MoveOrientation_e::BACKWARD);
            playerComp->m_inMovement = true;
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
        playerComp->m_playerAction = (glfwGetKey(m_window, GLFW_KEY_SPACE) ==
                                      GLFW_PRESS) ?
                    true : false;
        if (glfwGetKey(m_window, GLFW_KEY_M) == GLFW_PRESS)
        {
            m_mainEngine->setUnsetPaused();
        }
        if(!playerComp->m_weaponChange && !playerComp->m_timerShootActive)
        {
            //Change weapon
            if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS)
            {
                changePlayerWeapon(*playerComp, false);
            }
            else if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS)
            {
                changePlayerWeapon(*playerComp, true);
            }
            else if(!playerComp->m_timerShootActive)
            {
                if(glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                {
                    playerComp->m_playerShoot = true;
                    m_mainEngine->shoot(playerComp, mapComp->m_absoluteMapPositionPX,
                                        moveComp->m_degreeOrientation,
                                        CollisionTag_e::BULLET_PLAYER_CT);
                }
            }
        }
    }
}

//===================================================================
void InputSystem::treatMainMenu(uint32_t playerEntity)
{
    if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        m_keyEspapePressed = true;
        m_mainEngine->setUnsetPaused();
        return;
    }
    if(m_keyUpPressed && glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_RELEASE)
    {
        m_keyUpPressed = false;
        return;
    }
    if(m_keyDownPressed && glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_RELEASE)
    {
        m_keyDownPressed = false;
        return;
    }
    PlayerConfComponent *playerComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(playerEntity,
                                                       Components_e::PLAYER_CONF_COMPONENT);
    assert(playerComp);
    if(!m_keyUpPressed && glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        m_keyUpPressed = true;
        uint32_t index = static_cast<uint32_t>(playerComp->m_currentCursorPos);
        if(index == 0)
        {
            playerComp->m_currentCursorPos = static_cast<CurrentMenuCursorPos_e>(m_maxMenuCursorIndex);
        }
        else
        {
            playerComp->m_currentCursorPos = static_cast<CurrentMenuCursorPos_e>(index - 1);
        }
    }
    else if(!m_keyDownPressed && glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        m_keyDownPressed = true;
        uint32_t index = static_cast<uint32_t>(playerComp->m_currentCursorPos);
        if(index == m_maxMenuCursorIndex)
        {
            playerComp->m_currentCursorPos = static_cast<CurrentMenuCursorPos_e>(0);
        }
        else
        {
            playerComp->m_currentCursorPos = static_cast<CurrentMenuCursorPos_e>(index + 1);
        }
    }
}

//===================================================================
void changePlayerWeapon(PlayerConfComponent &playerComp, bool next)
{
    playerComp.m_weaponChange = true;
    if(!next)
    {
        //first weapon
        if(playerComp.m_currentWeapon == WeaponsType_e::GUN)
        {
            playerComp.m_currentWeapon = WeaponsType_e::SHOTGUN;
        }
        else
        {
            playerComp.m_currentWeapon = static_cast<WeaponsType_e>(
                        static_cast<uint8_t>(playerComp.m_currentWeapon) - 1);
        }
    }
    else if(next)
    {
        //last weapon
        if(playerComp.m_currentWeapon == WeaponsType_e::SHOTGUN)
        {
            playerComp.m_currentWeapon = WeaponsType_e::GUN;
        }
        else
        {
            playerComp.m_currentWeapon = static_cast<WeaponsType_e>(
                        static_cast<uint8_t>(playerComp.m_currentWeapon) + 1);
        }
    }
}

//===================================================================
void InputSystem::execSystem()
{
    System::execSystem();
    treatPlayerInput();
}
