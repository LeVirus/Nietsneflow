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
#include <ECS/Systems/StaticDisplaySystem.hpp>
#include "PhysicalEngine.hpp"
#include <MainEngine.hpp>
#include <cassert>


//===================================================================
InputSystem::InputSystem()
{
    setUsedComponents();
    gamepadInit();
}

//===================================================================
void InputSystem::gamepadInit()
{
    m_vectGamepadID.clear();
    for(uint32_t i = 0; i <= GLFW_JOYSTICK_LAST; ++i)
    {
        if(glfwJoystickPresent(i) == GLFW_TRUE)
        {
            m_vectGamepadID.insert({i, {nullptr, nullptr}});
        }
    }
}

//===================================================================
bool InputSystem::checkStandardButtonGamepadKeyStatus(uint32_t key, uint32_t status)
{
    //pair const uint8_t* :: buttons, const float* :: axes
    for(MapGamepadInputData_t::const_iterator it = m_vectGamepadID.begin(); it != m_vectGamepadID.end(); ++it)
    {
        if(!it->second.first)
        {
            return false;
        }
        if(it->second.first[key] == status)
        {
            return true;
        }
    }
    return false;
}

//===================================================================
bool InputSystem::checkAxisGamepadKeyStatus(uint32_t key, bool positive)
{
    bool ok;
    for(MapGamepadInputData_t::const_iterator it = m_vectGamepadID.begin(); it != m_vectGamepadID.end(); ++it)
    {
        if(!it->second.second)
        {
            return false;
        }
        ok = positive ? it->second.second[key] >= 0.3f : it->second.second[key] <= -0.3f;
        if(ok)
        {
            return true;
        }
    }
    return false;
}

//===================================================================
void InputSystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::INPUT_COMPONENT);
}

//===================================================================
void InputSystem::getGamepadInputs()
{
    int count;
    for(MapGamepadInputData_t::iterator it = m_vectGamepadID.begin(); it != m_vectGamepadID.end(); ++it)
    {
       it->second.first = glfwGetJoystickButtons(it->first, &count);
       it->second.second = glfwGetJoystickAxes(it->first, &count);
    }
}

//===================================================================
void InputSystem::treatPlayerInput()
{
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        getGamepadInputs();
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
        if(checkPlayerKeyTriggered(ControlKey_e::TURN_RIGHT))
        {
            moveComp->m_degreeOrientation -= moveComp->m_rotationAngle;
            if(moveComp->m_degreeOrientation < 0.0f)
            {
                moveComp->m_degreeOrientation += 360.0f;
            }
            updatePlayerOrientation(*moveComp, *posComp, *visionComp);
        }
        else if(checkPlayerKeyTriggered(ControlKey_e::TURN_LEFT))
        {
            moveComp->m_degreeOrientation += moveComp->m_rotationAngle;
            if(moveComp->m_degreeOrientation > 360.0f)
            {
                moveComp->m_degreeOrientation -= 360.0f;
            }
            updatePlayerOrientation(*moveComp, *posComp, *visionComp);
        }
        if(checkPlayerKeyTriggered(ControlKey_e::ACTION))
        {
            MapCoordComponent *mapCompAction = stairwayToComponentManager().
                    searchComponentByType<MapCoordComponent>(playerComp->m_actionEntity, Components_e::MAP_COORD_COMPONENT);
            assert(mapCompAction);
            GeneralCollisionComponent *genCompAction = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(playerComp->m_actionEntity, Components_e::GENERAL_COLLISION_COMPONENT);
            assert(genCompAction);
            confActionShape(mapCompAction, genCompAction, mapComp, moveComp);
        }
        if(glfwGetKey(m_window, GLFW_KEY_M) == GLFW_PRESS || checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_START, GLFW_PRESS))
        {
            playerComp->m_menuMode = MenuMode_e::BASE;
            m_mainEngine->setUnsetPaused();
        }
        if(!weaponComp->m_weaponChange && !weaponComp->m_timerShootActive)
        {
            //Change weapon
            if(checkPlayerKeyTriggered(ControlKey_e::PREVIOUS_WEAPON))
            {
                changePlayerWeapon(*weaponComp, false);
            }
            else if(checkPlayerKeyTriggered(ControlKey_e::NEXT_WEAPON))
            {
                changePlayerWeapon(*weaponComp, true);
            }
            //SHOOT
            else if(checkPlayerKeyTriggered(ControlKey_e::SHOOT))
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
bool InputSystem::checkPlayerKeyTriggered(ControlKey_e key)
{
    //KEYBOARD
    if(glfwGetKey(m_window, m_mapKeyboardCurrentAssociatedKey[key]) == GLFW_PRESS)
    {
        return true;
    }
    //GAMEPAD
    if(m_mapGamepadCurrentAssociatedKey[key].m_standardButton)
    {
        if(checkStandardButtonGamepadKeyStatus(m_mapGamepadCurrentAssociatedKey[key].m_keyID, GLFW_PRESS))
        {
            return true;
        }
    }
    else
    {
        assert(m_mapGamepadCurrentAssociatedKey[key].m_axesPos);
        if(checkAxisGamepadKeyStatus(m_mapGamepadCurrentAssociatedKey[key].m_keyID, *m_mapGamepadCurrentAssociatedKey[key].m_axesPos))
        {
            return true;
        }
    }
    return false;
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
    if(checkPlayerKeyTriggered(ControlKey_e::STRAFE_RIGHT))
    {
        currentMoveDirection = MoveOrientation_e::RIGHT;
        playerComp->m_inMovement = true;
    }
    else if(checkPlayerKeyTriggered(ControlKey_e::STRAFE_LEFT))
    {
        currentMoveDirection = MoveOrientation_e::LEFT;
        playerComp->m_inMovement = true;
    }
    if(checkPlayerKeyTriggered(ControlKey_e::MOVE_FORWARD))
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
    else if(checkPlayerKeyTriggered(ControlKey_e::MOVE_BACKWARD))
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
    if(checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_A, GLFW_RELEASE))
    {
        m_keyGamepadButtonAPressed = false;
    }
    if(m_enterPressed || m_keyGamepadButtonAPressed)
    {
        return;
    }
    if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
    {
        m_keyEspapePressed = false;
    }
    if(checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_B, GLFW_RELEASE))
    {
        m_keyGamepadButtonBPressed = false;
    }
    if((!m_keyEspapePressed && glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) ||
            (!m_keyGamepadButtonBPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_B, GLFW_PRESS)))
    {
        m_keyEspapePressed = true;
        m_keyGamepadButtonBPressed = true;
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
    if(playerComp->m_menuMode == MenuMode_e::NEW_KEY)
    {
        if(treatNewKey(playerComp))
        {
            playerComp->m_menuMode = MenuMode_e::INPUT;
            m_mainEngine->setMenuEntries(playerComp);
        }
    }
    treatReleaseInputMenu();
    treatGeneralKeysMenu(playerComp);
}

//===================================================================
void InputSystem::treatReleaseInputMenu()
{
    //UP
    if(m_keyUpPressed && glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_RELEASE)
    {
        m_keyUpPressed = false;
    }
    if(m_keyGamepadUpPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_RELEASE))
    {
        m_keyGamepadUpPressed = false;
    }
    //DOWN
    if(m_keyDownPressed && glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_RELEASE)
    {
        m_keyDownPressed = false;
    }
    if(m_keyGamepadDownPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_RELEASE))
    {
        m_keyGamepadDownPressed = false;
    }
    //LEFT
    if(m_keyLeftPressed && glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_RELEASE)
    {
        m_keyLeftPressed = false;
    }
    if(m_keyGamepadLeftPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_RELEASE))
    {
        m_keyGamepadLeftPressed = false;
    }
    //RIGHT
    if(m_keyRightPressed && glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
    {
        m_keyRightPressed = false;
    }
    if(m_keyGamepadRightPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_RELEASE))
    {
        m_keyGamepadRightPressed = false;
    }
    //TOOGLE GAMEPAD KEYBOARD INPUT MENU
    if(m_keyKeyboardGPressed && glfwGetKey(m_window, GLFW_KEY_G) == GLFW_RELEASE)
    {
        m_keyKeyboardGPressed = false;
    }
    if(m_keyGamepadButtonRightBumperPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_RELEASE))
    {
        m_keyGamepadButtonRightBumperPressed = false;
    }
}

//===================================================================
void InputSystem::treatGeneralKeysMenu(PlayerConfComponent *playerComp)
{
    uint32_t maxMenuIndex = m_mapMenuSize.at(playerComp->m_menuMode);
    if(!m_modeTransition && ((!m_keyUpPressed && glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) ||
                             (!m_keyGamepadUpPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_PRESS))))
    {
        m_keyUpPressed = true;
        m_keyGamepadUpPressed = true;
        uint32_t index = playerComp->m_currentCursorPos;
        if(index == 0)
        {
            playerComp->m_currentCursorPos = maxMenuIndex;
        }
        else
        {
            playerComp->m_currentCursorPos = index - 1;
        }
    }
    else if(!m_modeTransition && ((!m_keyDownPressed && (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)) ||
                                  (!m_keyGamepadDownPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_PRESS))))
    {
        m_keyDownPressed = true;
        m_keyGamepadDownPressed = true;
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
    else if(glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_PRESS || checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_A, GLFW_PRESS))
    {
        treatEnterPressedMenu(playerComp);
    }
    else if((!m_keyLeftPressed && (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)) ||
             (!m_keyGamepadLeftPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_PRESS)))
    {
        treatLeftPressedMenu(playerComp);
    }
    else if((!m_keyRightPressed && (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)) ||
             (!m_keyGamepadRightPressed && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_PRESS)))
    {
        treatRightPressedMenu(playerComp);
    }
    else if(!m_keyKeyboardGPressed && !m_keyGamepadButtonRightBumperPressed &&
            playerComp->m_menuMode == MenuMode_e::INPUT && (glfwGetKey(m_window, GLFW_KEY_G) == GLFW_PRESS ||
                                                            checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_PRESS)))
    {
        toogleInputMenuGamepadKeyboard(playerComp);
    }
}

//===================================================================
void InputSystem::toogleInputMenuGamepadKeyboard(PlayerConfComponent *playerComp)
{
    m_keyKeyboardGPressed = true;
    m_keyGamepadButtonRightBumperPressed = true;
    playerComp->m_keyboardInputMenuMode = !playerComp->m_keyboardInputMenuMode;
    m_mainEngine->updateInputMenuInfo(playerComp);
    mptrSystemManager->searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM))->
            updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode);
}

//===================================================================
bool InputSystem::treatNewKey(PlayerConfComponent *playerComp)
{
    StaticDisplaySystem *staticSystem = mptrSystemManager->searchSystemByType<StaticDisplaySystem>(
                static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    if(playerComp->m_keyboardInputMenuMode)
    {
        const std::map<uint32_t, std::string> &map = staticSystem->getKeyboardInputKeys();
        for(std::map<uint32_t, std::string>::const_iterator it = map.begin(); it != map.end(); ++it)
        {
            if(glfwGetKey(m_window, it->first) == GLFW_PRESS)
            {
                m_mapKeyboardTmpAssociatedKey[m_currentSelectedKey] = it->first;
                staticSystem->updateNewInputKey(m_currentSelectedKey, it->first, true);
                return true;
            }
        }
    }
    else
    {
        //BUTTONS
        const std::map<uint32_t, std::string> &mapButtons = staticSystem->getGamepadButtonsInputKeys();
        for(std::map<uint32_t, std::string>::const_iterator it = mapButtons.begin(); it != mapButtons.end(); ++it)
        {
            if(checkStandardButtonGamepadKeyStatus(it->first, GLFW_PRESS))
            {
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_standardButton = true;
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_axesPos = {};
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_keyID = it->first;
                staticSystem->updateNewInputKey(m_currentSelectedKey, it->first, false);
                return true;
            }
        }
        //AXES
        const std::map<uint32_t, std::string> &mapAxes = staticSystem->getGamepadAxesInputKeys();
        for(std::map<uint32_t, std::string>::const_iterator it = mapAxes.begin(); it != mapAxes.end(); ++it)
        {
            if(checkAxisGamepadKeyStatus(it->first, true))
            {
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_standardButton = false;
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_axesPos = true;
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_keyID = it->first;
                staticSystem->updateNewInputKey(m_currentSelectedKey, it->first, false);
                return true;
            }
        }
    }
    return false;
}

//===================================================================
void InputSystem::treatEnterPressedMenu(PlayerConfComponent *playerComp)
{
    m_enterPressed = true;
    m_keyGamepadButtonAPressed = true;
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
        m_keyGamepadLeftPressed = true;
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
        m_keyGamepadRightPressed = true;
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
        m_mapKeyboardTmpAssociatedKey = m_mapKeyboardCurrentAssociatedKey;
        m_mainEngine->updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode);
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
        m_currentSelectedKey = m_mapInputControl.at(menuPos);
    }
    else if(menuPos == InputMenuCursorPos_e::RETURN)
    {
        playerComp->m_menuMode = MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
    }
    else if(menuPos == InputMenuCursorPos_e::DEFAULT)
    {
        if(playerComp->m_keyboardInputMenuMode)
        {
            m_mapKeyboardTmpAssociatedKey = m_mapKeyboardCurrentAssociatedKey;
            m_mapKeyboardCurrentAssociatedKey = m_mapKeyboardDefaultAssociatedKey;
            m_mainEngine->updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode);
            m_mapKeyboardCurrentAssociatedKey = m_mapKeyboardTmpAssociatedKey;
            m_mapKeyboardTmpAssociatedKey = m_mapKeyboardDefaultAssociatedKey;
        }
        else
        {
            m_mapGamepadTmpAssociatedKey = m_mapGamepadCurrentAssociatedKey;
            m_mapGamepadCurrentAssociatedKey = m_mapGamepadDefaultAssociatedKey;
            m_mainEngine->updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode);
            m_mapGamepadCurrentAssociatedKey = m_mapGamepadTmpAssociatedKey;
            m_mapGamepadTmpAssociatedKey = m_mapGamepadDefaultAssociatedKey;
        }
    }
    else if(menuPos == InputMenuCursorPos_e::VALID)
    {
        if(playerComp->m_keyboardInputMenuMode)
        {
            m_mapKeyboardCurrentAssociatedKey = m_mapKeyboardTmpAssociatedKey;
        }
        else
        {
            m_mapGamepadCurrentAssociatedKey = m_mapGamepadTmpAssociatedKey;
        }
        m_mainEngine->updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode);
        playerComp->m_menuMode = MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
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
