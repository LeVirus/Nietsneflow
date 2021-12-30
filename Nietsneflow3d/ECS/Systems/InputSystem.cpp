#include "InputSystem.hpp"
#include "constants.hpp"
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/WriteComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/AudioComponent.hpp>
#include <ECS/Components/WeaponComponent.hpp>
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
            treatMenu(mVectNumEntity[i]);
            continue;
        }
        if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
        {
            m_keyEspapePressed = false;
        }
        else if(!m_keyEspapePressed && glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(m_window, true);
            return;
        }
        if(!m_F12Pressed && glfwGetKey(m_window, GLFW_KEY_F12) == GLFW_PRESS)
        {
            m_toggleSignal = true;
            m_F12Pressed = true;
        }
        else if(m_F12Pressed)
        {
            m_F12Pressed = false;
        }
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        assert(mapComp);
        MoveableComponent *moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(mVectNumEntity[i], Components_e::MOVEABLE_COMPONENT);
        assert(moveComp);
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(mVectNumEntity[i], Components_e::POSITION_VERTEX_COMPONENT);
        assert(posComp);
        VisionComponent *visionComp = stairwayToComponentManager().
                searchComponentByType<VisionComponent>(mVectNumEntity[i], Components_e::VISION_COMPONENT);
        assert(visionComp);
        PlayerConfComponent *playerComp = stairwayToComponentManager().
                searchComponentByType<PlayerConfComponent>(mVectNumEntity[i], Components_e::PLAYER_CONF_COMPONENT);
        assert(playerComp);
        WeaponComponent *weaponComp = stairwayToComponentManager().
                searchComponentByType<WeaponComponent>(playerComp->m_weaponEntity, Components_e::WEAPON_COMPONENT);
        assert(weaponComp);
        treatPlayerMove(playerComp, moveComp, mapComp);
        if(glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            moveComp->m_degreeOrientation -= moveComp->m_rotationAngle;
            if(moveComp->m_degreeOrientation < 0.0f)
            {
                moveComp->m_degreeOrientation += 360.0f;
            }
            updatePlayerOrientation(*moveComp, *posComp, *visionComp);
        }
        else if(glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            moveComp->m_degreeOrientation += moveComp->m_rotationAngle;
            if(moveComp->m_degreeOrientation > 360.0f)
            {
                moveComp->m_degreeOrientation -= 360.0f;
            }
            updatePlayerOrientation(*moveComp, *posComp, *visionComp);
        }
        if(glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            MapCoordComponent *mapCompAction = stairwayToComponentManager().
                    searchComponentByType<MapCoordComponent>(playerComp->m_actionEntity, Components_e::MAP_COORD_COMPONENT);
            assert(mapCompAction);
            GeneralCollisionComponent *genCompAction = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(playerComp->m_actionEntity, Components_e::GENERAL_COLLISION_COMPONENT);
            assert(genCompAction);
            confActionShape(mapCompAction, genCompAction, mapComp, moveComp);
        }
        if(glfwGetKey(m_window, GLFW_KEY_M) == GLFW_PRESS)
        {
            playerComp->m_menuMode = MenuMode_e::BASE;
            m_mainEngine->setUnsetPaused();
        }
        if(!weaponComp->m_weaponChange && !weaponComp->m_timerShootActive)
        {
            //Change weapon
            if(glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS)
            {
                changePlayerWeapon(*weaponComp, false);
            }
            else if(glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS)
            {
                changePlayerWeapon(*weaponComp, true);
            }
            //SHOOT
            else if(glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            {
                if(!weaponComp->m_timerShootActive && weaponComp->m_weaponsData[weaponComp->m_currentWeapon].m_ammunationsCount > 0)
                {
                    playerComp->m_playerShoot = true;
                    AudioComponent *audioComp = stairwayToComponentManager().
                            searchComponentByType<AudioComponent>(playerComp->m_weaponEntity, Components_e::AUDIO_COMPONENT);
                    assert(audioComp);
                    audioComp->m_soundElements[weaponComp->m_currentWeapon]->m_toPlay = true;
                    m_mainEngine->playerAttack(mVectNumEntity[i], playerComp, mapComp->m_absoluteMapPositionPX, moveComp->m_degreeOrientation);
                    if(weaponComp->m_weaponsData[weaponComp->m_currentWeapon].m_ammunationsCount == 0)
                    {
                        changeToTopPlayerWeapon(*weaponComp);
                        playerComp->m_playerShoot = false;
                    }
                }
                else
                {
                    changePlayerWeapon(*weaponComp, false);
                }
            }
        }
    }
}

//===================================================================
void InputSystem::treatPlayerMove(PlayerConfComponent *playerComp, MoveableComponent *moveComp,
                                  MapCoordComponent *mapComp)
{
    playerComp->m_inMovement = false;
    if(playerComp->m_life == 0)
    {
        return;
    }
    //init value
    MoveOrientation_e currentMoveDirection = MoveOrientation_e::FORWARD;
    //STRAFE
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    {
        currentMoveDirection = MoveOrientation_e::RIGHT;
        playerComp->m_inMovement = true;
    }
    else if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        currentMoveDirection = MoveOrientation_e::LEFT;
        playerComp->m_inMovement = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        if(currentMoveDirection == MoveOrientation_e::RIGHT)
        {
            currentMoveDirection = MoveOrientation_e::FORWARD_RIGHT;
        }
        else if(currentMoveDirection == MoveOrientation_e::LEFT)
        {
            currentMoveDirection = MoveOrientation_e::FORWARD_LEFT;
        }
        else
        {
            currentMoveDirection = MoveOrientation_e::FORWARD;
        }
        playerComp->m_inMovement = true;
    }
    else if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        if(currentMoveDirection == MoveOrientation_e::RIGHT)
        {
            currentMoveDirection = MoveOrientation_e::BACKWARD_RIGHT;
        }
        else if(currentMoveDirection == MoveOrientation_e::LEFT)
        {
            currentMoveDirection = MoveOrientation_e::BACKWARD_LEFT;
        }
        else
        {
            currentMoveDirection = MoveOrientation_e::BACKWARD;
        }
        playerComp->m_inMovement = true;
    }
    if(playerComp->m_inMovement && !playerComp->m_frozen)
    {
        moveComp->m_currentDegreeMoveDirection = moveComp->m_degreeOrientation;
        switch(currentMoveDirection)
        {
        case MoveOrientation_e::FORWARD:
            break;
        case MoveOrientation_e::FORWARD_LEFT:
            moveComp->m_currentDegreeMoveDirection += 45;
            break;
        case MoveOrientation_e::FORWARD_RIGHT:
            moveComp->m_currentDegreeMoveDirection += 315;
            break;
        case MoveOrientation_e::BACKWARD:
            moveComp->m_currentDegreeMoveDirection += 180;
            break;
        case MoveOrientation_e::BACKWARD_LEFT:
            moveComp->m_currentDegreeMoveDirection += 135;
            break;
        case MoveOrientation_e::BACKWARD_RIGHT:
            moveComp->m_currentDegreeMoveDirection += 225;
            break;
        case MoveOrientation_e::LEFT:
            moveComp->m_currentDegreeMoveDirection += 90;
            break;
        case MoveOrientation_e::RIGHT:
            moveComp->m_currentDegreeMoveDirection += 270;
            break;
        }
        moveElementFromAngle(moveComp->m_velocity,
                             getRadiantAngle(moveComp->m_currentDegreeMoveDirection),
                             mapComp->m_absoluteMapPositionPX, true);
    }
}

//===================================================================
void InputSystem::treatMenu(uint32_t playerEntity)
{
    PlayerConfComponent *playerComp = stairwayToComponentManager().
            searchComponentByType<PlayerConfComponent>(playerEntity,
                                                       Components_e::PLAYER_CONF_COMPONENT);
    assert(playerComp);
    if(glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_RELEASE)
    {
        m_enterPressed = false;
    }
    if(m_enterPressed)
    {
        return;
    }
    if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
    {
        m_keyEspapePressed = false;
    }
    else if(!m_keyEspapePressed && glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        m_keyEspapePressed = true;
        if(playerComp->m_menuMode == MenuMode_e::BASE || playerComp->m_menuMode == MenuMode_e::TRANSITION_LEVEL)
        {
            m_mainEngine->setUnsetPaused();
        }
        else
        {
            playerComp->m_currentCursorPos = 0;
            playerComp->m_menuMode = MenuMode_e::BASE;
            m_mainEngine->setMenuEntries(MenuMode_e::BASE, playerComp);
            return;
        }
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
    uint32_t maxMenuIndex = m_mapMenuSize.at(playerComp->m_menuMode);
    if(!m_modeTransition && !m_keyUpPressed && glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        m_keyUpPressed = true;
        uint32_t index = playerComp->m_currentCursorPos;
        if(index == 0)
        {
            playerComp->m_currentCursorPos = maxMenuIndex ;
        }
        else
        {
            playerComp->m_currentCursorPos = index - 1;
        }
    }
    else if(!m_modeTransition && !m_keyDownPressed && glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        m_keyDownPressed = true;
        uint32_t index = playerComp->m_currentCursorPos;
        if(index == maxMenuIndex)
        {
            playerComp->m_currentCursorPos = 0;
        }
        else
        {
            playerComp->m_currentCursorPos = index + 1;
        }
    }
    else if(glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        switch (playerComp->m_menuMode)
        {
        case MenuMode_e::BASE:
            treatMainMenu(playerComp);
            break;
        case MenuMode_e::SOUND:
            treatSoundMenu(playerComp);
            break;
        case MenuMode_e::DISPLAY:
            treatDisplayMenu(playerComp);
            break;
        case MenuMode_e::INPUT:
            treatInputMenu(playerComp);
            break;
        case MenuMode_e::TRANSITION_LEVEL:
            m_mainEngine->setUnsetPaused();
            break;
        }
    }
}

//===================================================================
void InputSystem::treatMainMenu(PlayerConfComponent *playerComp)
{
    MainMenuCursorPos_e menuPos = static_cast<MainMenuCursorPos_e>(playerComp->m_currentCursorPos);
    switch(menuPos)
    {
    case MainMenuCursorPos_e::SOUND_CONF:
        playerComp->m_currentCursorPos = 0;
        playerComp->m_menuMode = MenuMode_e::SOUND;
        m_mainEngine->setMenuEntries(MenuMode_e::SOUND, playerComp);
        break;
    case MainMenuCursorPos_e::DISPLAY_CONF:
        playerComp->m_currentCursorPos = 0;
        playerComp->m_menuMode = MenuMode_e::DISPLAY;
        m_mainEngine->setMenuEntries(MenuMode_e::DISPLAY, playerComp);
        break;
    case MainMenuCursorPos_e::INPUT_CONF:
        playerComp->m_currentCursorPos = 0;
        playerComp->m_menuMode = MenuMode_e::INPUT;
        m_mainEngine->setMenuEntries(MenuMode_e::INPUT, playerComp);
        break;
    case MainMenuCursorPos_e::NEW_GAME:
        break;
    case MainMenuCursorPos_e::QUIT_GAME:
        glfwSetWindowShouldClose(m_window, true);
        break;
    case MainMenuCursorPos_e::RETURN_TO_GAME:
        m_mainEngine->setUnsetPaused();
        break;
    case MainMenuCursorPos_e::TOTAL:
        break;
    }
}

//===================================================================
void InputSystem::treatSoundMenu(PlayerConfComponent *playerComp)
{
    SoundMenuCursorPos_e menuPos = static_cast<SoundMenuCursorPos_e>(playerComp->m_currentCursorPos);
    switch(menuPos)
    {
    case SoundMenuCursorPos_e::MUSIC_VOLUME:
        break;
    case SoundMenuCursorPos_e::EFFECTS_VOLUME:
        break;
    case SoundMenuCursorPos_e::RETURN:
        playerComp->m_currentCursorPos = 0;
        playerComp->m_menuMode = MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(MenuMode_e::BASE, playerComp);
        m_enterPressed = true;
        break;
    case SoundMenuCursorPos_e::VALID:
        break;
    case SoundMenuCursorPos_e::TOTAL:
        break;
    }
}

//===================================================================
void InputSystem::treatDisplayMenu(PlayerConfComponent *playerComp)
{
    DisplayMenuCursorPos_e menuPos =
            static_cast<DisplayMenuCursorPos_e>(playerComp->m_currentCursorPos);
    switch(menuPos)
    {
    case DisplayMenuCursorPos_e::QUALITY_SETTING:
        break;
    case DisplayMenuCursorPos_e::RESOLUTION_SETTING:
        break;
    case DisplayMenuCursorPos_e::RETURN:
        playerComp->m_currentCursorPos = 0;
        playerComp->m_menuMode = MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(MenuMode_e::BASE, playerComp);
        m_enterPressed = true;
        break;
    case DisplayMenuCursorPos_e::VALID:
        break;
    case DisplayMenuCursorPos_e::TOTAL:
        break;
    }
}

//===================================================================
void InputSystem::treatInputMenu(PlayerConfComponent *playerComp)
{
    InputMenuCursorPos_e menuPos = static_cast<InputMenuCursorPos_e>(playerComp->m_currentCursorPos);
    switch(menuPos)
    {
    case InputMenuCursorPos_e::ACTION:
        break;
    case InputMenuCursorPos_e::MOVE_BACKWARD:
        break;
    case InputMenuCursorPos_e::MOVE_FORWARD:
        break;
    case InputMenuCursorPos_e::SHOOT:
        break;
    case InputMenuCursorPos_e::TURN_LEFT:
        break;
    case InputMenuCursorPos_e::TURN_RIGHT:
        break;
    case InputMenuCursorPos_e::STRAFE_LEFT:
        break;
    case InputMenuCursorPos_e::STRAFE_RIGHT:
        break;
    case InputMenuCursorPos_e::RETURN:
        playerComp->m_currentCursorPos = 0;
        playerComp->m_menuMode = MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(MenuMode_e::BASE, playerComp);
        m_enterPressed = true;
        break;
    case InputMenuCursorPos_e::VALID:
        break;
    case InputMenuCursorPos_e::DEFAULT:
        break;
    case InputMenuCursorPos_e::TOTAL:
        break;
    }
}

//===================================================================
void changePlayerWeapon(WeaponComponent &weaponComp, bool next)
{
    uint32_t weapon = weaponComp.m_currentWeapon;
    if(!next)
    {
        do
        {
            //first weapon
            if(weapon == 0)
            {
                weapon = weaponComp.m_weaponsData.size() - 1;
            }
            else
            {
                --weapon;
            }
            if(weaponComp.m_weaponsData[weapon].m_posses && (weaponComp.m_weaponsData[weapon].m_attackType == AttackType_e::MELEE ||
                                                             weaponComp.m_weaponsData[weapon].m_ammunationsCount > 0))
            {
                break;
            }
        }while(true);
    }
    else if(next)
    {
        do
        {
            //last weapon
            if(weapon == weaponComp.m_weaponsData.size() - 1)
            {
                weapon = 0;
            }
            else
            {
                ++weapon;
            }
            if(weaponComp.m_weaponsData[weapon].m_posses && (weaponComp.m_weaponsData[weapon].m_attackType == AttackType_e::MELEE ||
                                                             weaponComp.m_weaponsData[weapon].m_ammunationsCount > 0))
            {
                break;
            }
        }while(true);
    }
    setPlayerWeapon(weaponComp, weapon);
}

//===================================================================
void changeToTopPlayerWeapon(WeaponComponent &weaponComp)
{
    uint32_t max = 0;
    for(uint32_t i = 0; i < weaponComp.m_weaponsData.size(); ++i)
    {
        if(weaponComp.m_weaponsData[i].m_posses && (weaponComp.m_weaponsData[i].m_attackType == AttackType_e::MELEE ||
                                                    weaponComp.m_weaponsData[i].m_ammunationsCount > 0))
        {
            max = i;
        }
    }
    setPlayerWeapon(weaponComp, max);
}

//===================================================================
void setPlayerWeapon(WeaponComponent &weaponComp, uint32_t weapon)
{
    weaponComp.m_timerShootActive = false;
    if(weaponComp.m_currentWeapon != weapon)
    {
        weaponComp.m_currentWeapon = weapon;
        weaponComp.m_weaponChange = true;
    }
}

//===================================================================
void InputSystem::execSystem()
{
    System::execSystem();
    treatPlayerInput();
}
