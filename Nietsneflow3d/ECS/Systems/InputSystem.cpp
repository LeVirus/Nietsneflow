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
        if(!m_F12Pressed && glfwGetKey(m_window, GLFW_KEY_F12) == GLFW_PRESS)
        {
            m_toggleSignal = true;
            m_F12Pressed = true;
        }
        else if(m_F12Pressed && glfwGetKey(m_window, GLFW_KEY_F12) == GLFW_RELEASE)
        {
            m_F12Pressed = false;
        }
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
        if(glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::TURN_RIGHT]) == GLFW_PRESS)
        {
            moveComp->m_degreeOrientation -= moveComp->m_rotationAngle;
            if(moveComp->m_degreeOrientation < 0.0f)
            {
                moveComp->m_degreeOrientation += 360.0f;
            }
            updatePlayerOrientation(*moveComp, *posComp, *visionComp);
        }
        else if(glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::TURN_LEFT]) == GLFW_PRESS)
        {
            moveComp->m_degreeOrientation += moveComp->m_rotationAngle;
            if(moveComp->m_degreeOrientation > 360.0f)
            {
                moveComp->m_degreeOrientation -= 360.0f;
            }
            updatePlayerOrientation(*moveComp, *posComp, *visionComp);
        }
        if(glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::ACTION]) == GLFW_PRESS)
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
            if(glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::PREVIOUS_WEAPON]) == GLFW_PRESS)
            {
                changePlayerWeapon(*weaponComp, false);
            }
            else if(glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::NEXT_WEAPON]) == GLFW_PRESS)
            {
                changePlayerWeapon(*weaponComp, true);
            }
            //SHOOT
            else if(glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::SHOOT]) == GLFW_PRESS)
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
    if (glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::STRAFE_RIGHT]) == GLFW_PRESS)
    {
        currentMoveDirection = MoveOrientation_e::RIGHT;
        playerComp->m_inMovement = true;
    }
    else if (glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::STRAFE_LEFT]) == GLFW_PRESS)
    {
        currentMoveDirection = MoveOrientation_e::LEFT;
        playerComp->m_inMovement = true;
    }
    if (glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::MOVE_FORWARD]) == GLFW_PRESS)
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
    else if (glfwGetKey(m_window, m_mapCurrentAssociatedKey[ControlKey_e::MOVE_BACKWARD]) == GLFW_PRESS)
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
            searchComponentByType<PlayerConfComponent>(playerEntity, Components_e::PLAYER_CONF_COMPONENT);
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
        else if(playerComp->m_menuMode == MenuMode_e::NEW_KEY)
        {
            playerComp->m_menuMode = MenuMode_e::INPUT;
            m_mainEngine->setMenuEntries(playerComp);
        }
        else
        {
            playerComp->m_menuMode = MenuMode_e::BASE;
            m_mainEngine->setMenuEntries(playerComp);
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
    if(m_keyLeftPressed && glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_RELEASE)
    {
        m_keyLeftPressed = false;
        return;
    }
    if(m_keyRightPressed && glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
    {
        m_keyRightPressed = false;
        return;
    }
    treatGeneralKeysMenu(playerComp);
}

//===================================================================
void InputSystem::treatGeneralKeysMenu(PlayerConfComponent *playerComp)
{
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
        treatEnterPressedMenu(playerComp);
    }
    else if(!m_keyLeftPressed && glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        treatLeftPressedMenu(playerComp);
    }
    else if(!m_keyRightPressed && glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        treatRightPressedMenu(playerComp);
    }
}

//===================================================================
void InputSystem::treatEnterPressedMenu(PlayerConfComponent *playerComp)
{
    m_enterPressed = true;
    switch (playerComp->m_menuMode)
    {
    case MenuMode_e::BASE:
        treatEnterPressedMainMenu(playerComp);
        break;
    case MenuMode_e::SOUND:
        treatEnterPressedSoundMenu(playerComp);
        break;
    case MenuMode_e::DISPLAY:
        treatEnterPressedDisplayMenu(playerComp);
        break;
    case MenuMode_e::INPUT:
        treatEnterPressedInputMenu(playerComp);
        break;
    case MenuMode_e::TRANSITION_LEVEL:
        m_mainEngine->setUnsetPaused();
        break;
    case MenuMode_e::NEW_KEY:
        break;
    }
}

//===================================================================
void InputSystem::treatLeftPressedMenu(PlayerConfComponent *playerComp)
{
    if(playerComp->m_menuMode == MenuMode_e::SOUND)
    {
        SoundMenuCursorPos_e soundCursorPos = static_cast<SoundMenuCursorPos_e>(playerComp->m_currentCursorPos);
        if(soundCursorPos == SoundMenuCursorPos_e::MUSIC_VOLUME)
        {
            uint32_t musicVolume = m_mainEngine->getMusicVolume();
            if(musicVolume > 0)
            {
                m_mainEngine->updateMusicVolume(--musicVolume);
            }
        }
        else if(soundCursorPos == SoundMenuCursorPos_e::EFFECTS_VOLUME)
        {
            uint32_t effectsVolume = m_mainEngine->getEffectsVolume();
            if(effectsVolume > 0)
            {
                m_mainEngine->updateEffectsVolume(--effectsVolume);
            }
        }
    }
    else if(playerComp->m_menuMode == MenuMode_e::DISPLAY)
    {
        m_keyLeftPressed = true;
        DisplayMenuCursorPos_e displayCursorPos = static_cast<DisplayMenuCursorPos_e>(playerComp->m_currentCursorPos);
        if(displayCursorPos == DisplayMenuCursorPos_e::RESOLUTION_SETTING)
        {
            uint32_t index = m_mainEngine->getCurrentDisplayedResolutionNum();
            if(index == 0)
            {
                m_mainEngine->setCurrentResolution(m_mainEngine->getMaxResolutionNum());
            }
            else
            {
                m_mainEngine->setCurrentResolution(--index);
            }
        }
        else if(displayCursorPos == DisplayMenuCursorPos_e::QUALITY_SETTING)
        {
            m_mainEngine->decreaseDisplayQuality();
        }
        else if(displayCursorPos == DisplayMenuCursorPos_e::FULLSCREEN)
        {
            m_mainEngine->toogleMenuEntryFullscreen();
        }
    }
}

//===================================================================
void InputSystem::treatRightPressedMenu(PlayerConfComponent *playerComp)
{
    if(playerComp->m_menuMode == MenuMode_e::SOUND)
    {
        SoundMenuCursorPos_e soundCursorPos = static_cast<SoundMenuCursorPos_e>(playerComp->m_currentCursorPos);
        if(soundCursorPos == SoundMenuCursorPos_e::MUSIC_VOLUME)
        {
            uint32_t musicVolume = m_mainEngine->getMusicVolume();
            if(musicVolume < 100)
            {
                m_mainEngine->updateMusicVolume(++musicVolume);
            }
        }
        else if(soundCursorPos == SoundMenuCursorPos_e::EFFECTS_VOLUME)
        {
            uint32_t effectsVolume = m_mainEngine->getEffectsVolume();
            if(effectsVolume < 100)
            {
                m_mainEngine->updateEffectsVolume(++effectsVolume);
            }
        }
    }
    else if(playerComp->m_menuMode == MenuMode_e::DISPLAY)
    {
        m_keyRightPressed = true;
        DisplayMenuCursorPos_e displayCursorPos = static_cast<DisplayMenuCursorPos_e>(playerComp->m_currentCursorPos);
        if(displayCursorPos == DisplayMenuCursorPos_e::RESOLUTION_SETTING)
        {
            uint32_t index = m_mainEngine->getCurrentDisplayedResolutionNum();
            if(index == m_mainEngine->getMaxResolutionNum())
            {
                m_mainEngine->setCurrentResolution(0);
            }
            else
            {
                m_mainEngine->setCurrentResolution(++index);
            }
        }
        else if(displayCursorPos == DisplayMenuCursorPos_e::QUALITY_SETTING)
        {
            m_mainEngine->increaseDisplayQuality();
        }
        else if(displayCursorPos == DisplayMenuCursorPos_e::FULLSCREEN)
        {
            m_mainEngine->toogleMenuEntryFullscreen();
        }
    }
}

//===================================================================
void InputSystem::treatEnterPressedMainMenu(PlayerConfComponent *playerComp)
{
    MainMenuCursorPos_e menuPos = static_cast<MainMenuCursorPos_e>(playerComp->m_currentCursorPos);
    switch(menuPos)
    {
    case MainMenuCursorPos_e::SOUND_CONF:
        playerComp->m_menuMode = MenuMode_e::SOUND;
        m_mainEngine->setMenuEntries(playerComp);
        m_mainEngine->getMusicVolume();
        break;
    case MainMenuCursorPos_e::DISPLAY_CONF:
        playerComp->m_menuMode = MenuMode_e::DISPLAY;
        m_mainEngine->setMenuEntries(playerComp);
        break;
    case MainMenuCursorPos_e::INPUT_CONF:
        playerComp->m_menuMode = MenuMode_e::INPUT;
        m_mainEngine->setMenuEntries(playerComp);
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
void InputSystem::treatEnterPressedSoundMenu(PlayerConfComponent *playerComp)
{
    SoundMenuCursorPos_e menuPos = static_cast<SoundMenuCursorPos_e>(playerComp->m_currentCursorPos);
    switch(menuPos)
    {
    case SoundMenuCursorPos_e::MUSIC_VOLUME:
        break;
    case SoundMenuCursorPos_e::EFFECTS_VOLUME:
        break;
    case SoundMenuCursorPos_e::RETURN:
        playerComp->m_menuMode = MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
        break;
    case SoundMenuCursorPos_e::TOTAL:
        break;
    }
}

//===================================================================
void InputSystem::treatEnterPressedDisplayMenu(PlayerConfComponent *playerComp)
{
    DisplayMenuCursorPos_e menuPos =
            static_cast<DisplayMenuCursorPos_e>(playerComp->m_currentCursorPos);
    if(menuPos == DisplayMenuCursorPos_e::RETURN)
    {
        playerComp->m_menuMode = MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
    }
    else if(menuPos == DisplayMenuCursorPos_e::VALID)
    {
        m_mainEngine->validDisplayMenu();
    }
}

//===================================================================
void InputSystem::treatEnterPressedInputMenu(PlayerConfComponent *playerComp)
{
    InputMenuCursorPos_e menuPos = static_cast<InputMenuCursorPos_e>(playerComp->m_currentCursorPos);
    if(menuPos != InputMenuCursorPos_e::RETURN && menuPos != InputMenuCursorPos_e::VALID && menuPos != InputMenuCursorPos_e::DEFAULT)
    {
        playerComp->m_menuMode = MenuMode_e::NEW_KEY;
        m_mainEngine->setMenuEntries(playerComp);
        return;
    }
    if(menuPos == InputMenuCursorPos_e::RETURN)
    {
        playerComp->m_menuMode = MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
    }
//    switch(menuPos)
//    {
//    case InputMenuCursorPos_e::ACTION:
//        break;
//    case InputMenuCursorPos_e::MOVE_BACKWARD:
//        break;
//    case InputMenuCursorPos_e::MOVE_FORWARD:
//        break;
//    case InputMenuCursorPos_e::SHOOT:
//        break;
//    case InputMenuCursorPos_e::TURN_LEFT:
//        break;
//    case InputMenuCursorPos_e::TURN_RIGHT:
//        break;
//    case InputMenuCursorPos_e::STRAFE_LEFT:
//        break;
//    case InputMenuCursorPos_e::STRAFE_RIGHT:
//        break;
//    case InputMenuCursorPos_e::RETURN:
//        playerComp->m_menuMode = MenuMode_e::BASE;
//        m_mainEngine->setMenuEntries(playerComp);
//        break;
//    case InputMenuCursorPos_e::VALID:
//        break;
//    case InputMenuCursorPos_e::DEFAULT:
//        break;
//    case InputMenuCursorPos_e::TOTAL:
//        break;
//    }
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
