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

MapGamepadInputData_t InputSystem::m_mapGamepadID;
bool InputSystem::m_windowFocus;
bool InputSystem::m_scrollUp;
bool InputSystem::m_scrollDown;

//===================================================================
InputSystem::InputSystem()
{
    setUsedComponents();
    gamepadUpdate();
    glfwSetJoystickCallback(joystick_callback);
    m_windowFocus = true;
}

//===================================================================
void InputSystem::init(GLFWwindow &window)
{
    m_window = &window;
    glfwSetWindowFocusCallback(m_window, InputSystem::window_focus_callback);
    glfwSetScrollCallback(m_window, InputSystem::scroll_callback);
}

//===================================================================
void InputSystem::updateMousePos()
{
    glfwGetCursorPos(m_window, &m_previousMousePosition.first, &m_previousMousePosition.second);
}

//===================================================================
void InputSystem::gamepadUpdate()
{
    m_mapGamepadID.clear();
    for(uint32_t i = 0; i <= GLFW_JOYSTICK_LAST; ++i)
    {
        if(glfwJoystickPresent(i) == GLFW_TRUE)
        {
            m_mapGamepadID.insert({i, {nullptr, nullptr}});
        }
    }
    m_gamepadButtonsKeyPressed.fill(false);
}

//===================================================================
bool InputSystem::checkStandardButtonGamepadKeyStatus(uint32_t key, uint32_t status)
{
    //pair const uint8_t* :: buttons, const float* :: axis
    for(MapGamepadInputData_t::const_iterator it = m_mapGamepadID.begin(); it != m_mapGamepadID.end(); ++it)
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
    for(MapGamepadInputData_t::const_iterator it = m_mapGamepadID.begin(); it != m_mapGamepadID.end(); ++it)
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
    for(MapGamepadInputData_t::iterator it = m_mapGamepadID.begin(); it != m_mapGamepadID.end(); ++it)
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
        if(m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_B] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_B, GLFW_RELEASE))
        {
            m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_B] = false;
        }
        if(m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_A] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_A, GLFW_RELEASE))
        {
            m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_A] = false;
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
        PlayerConfComponent *playerComp = stairwayToComponentManager().
                searchComponentByType<PlayerConfComponent>(mVectNumEntity[i], Components_e::PLAYER_CONF_COMPONENT);
        assert(playerComp);
        if(!m_changeMapMode && glfwGetKey(m_window, GLFW_KEY_TAB) == GLFW_PRESS)
        {
            m_changeMapMode = true;
            if(playerComp->m_mapMode != MapMode_e::FULL_MAP)
            {
                uint32_t num = static_cast<uint32_t>(playerComp->m_mapMode);
                playerComp->m_mapMode = static_cast<MapMode_e>(++num);
            }
            else
            {
                playerComp->m_mapMode = MapMode_e::NONE;
            }

        }
        else if(m_changeMapMode && glfwGetKey(m_window, GLFW_KEY_TAB) == GLFW_RELEASE)
        {
            m_changeMapMode = false;
        }
        MapCoordComponent *mapComp = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        assert(mapComp);
        MoveableComponent *moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(mVectNumEntity[i], Components_e::MOVEABLE_COMPONENT);
        assert(moveComp);
        WeaponComponent *weaponComp = stairwayToComponentManager().
                searchComponentByType<WeaponComponent>(playerComp->m_weaponEntity, Components_e::WEAPON_COMPONENT);
        assert(weaponComp);
        treatPlayerMove(playerComp, moveComp, mapComp);
        std::optional<double> mouseXdiff = getXMouseMotion();
        if(checkPlayerKeyTriggered(ControlKey_e::TURN_RIGHT) ||
                (mouseXdiff && mouseXdiff > 0.00))
        {
            float diffBase = (moveComp->m_rotationAngle *
                              (m_rotationSensibility) / DIFF_TOTAL_SENSITIVITY);
            if(checkPlayerKeyTriggered(ControlKey_e::TURN_RIGHT))
            {
                moveComp->m_degreeOrientation -= diffBase * 2;
            }
            else
            {
                if(*mouseXdiff > 30.0)
                {
                    *mouseXdiff = 30.0;
                }
                moveComp->m_degreeOrientation -= diffBase * (*mouseXdiff) / 8;
            }
            if(moveComp->m_degreeOrientation < 0.0f)
            {
                moveComp->m_degreeOrientation += 360.0f;
            }
        }
        else if(checkPlayerKeyTriggered(ControlKey_e::TURN_LEFT) ||
                (mouseXdiff && mouseXdiff < 0.00))
        {
            float diffBase = (moveComp->m_rotationAngle *
                              (m_rotationSensibility) / DIFF_TOTAL_SENSITIVITY);
            if(checkPlayerKeyTriggered(ControlKey_e::TURN_LEFT))
            {
                moveComp->m_degreeOrientation += diffBase * 2;
            }
            else
            {
                if(*mouseXdiff < -30.0)
                {
                    *mouseXdiff = -30.0;
                }
                moveComp->m_degreeOrientation -= diffBase * (*mouseXdiff) / 8;
            }
            if(moveComp->m_degreeOrientation > 360.0f)
            {
                moveComp->m_degreeOrientation -= 360.0f;
            }
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
        if((!m_keyEspapePressed && glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) ||
                checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_START, GLFW_PRESS))
        {
            m_keyEspapePressed = true;
            playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
            m_mainEngine->setMenuEntries(playerComp);
            m_mainEngine->setUnsetPaused();
        }
        if(!weaponComp->m_weaponChange && !weaponComp->m_timerShootActive)
        {
            //Change weapon
            if(checkPlayerKeyTriggered(ControlKey_e::PREVIOUS_WEAPON) || m_scrollDown)
            {
                changePlayerWeapon(*weaponComp, false);
            }
            else if(checkPlayerKeyTriggered(ControlKey_e::NEXT_WEAPON) || m_scrollUp)
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
        m_scrollUp = false;
        m_scrollDown = false;
    }
}

//===================================================================
std::optional<double> InputSystem::getXMouseMotion()
{
    if(!m_windowFocus)
    {
        return {};
    }
    double xpos, ypos, xDiff;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    xDiff = xpos - m_previousMousePosition.first;
    if(std::abs(xDiff) < 0.01)
    {
        return {};
    }
    m_previousMousePosition.first = xpos;
    return xDiff;
}

//===================================================================
bool InputSystem::checkPlayerKeyTriggered(ControlKey_e key)
{
    //KEYBOARD
    if(glfwGetKey(m_window, m_mapKeyboardCurrentAssociatedKey[key].m_key) == GLFW_PRESS)
    {
        return true;
    }
    if(glfwGetMouseButton(m_window, m_mapKeyboardCurrentAssociatedKey[key].m_key) == GLFW_PRESS)
    {
        return true;
    }
    //GAMEPAD
    if(!m_mapGamepadID.empty())
    {
        if(m_mapGamepadCurrentAssociatedKey[key].m_standardButton)
        {
            if(m_gamepadButtonsKeyPressed[m_mapGamepadCurrentAssociatedKey[key].m_keyID])
            {
                return false;
            }
            if(checkStandardButtonGamepadKeyStatus(m_mapGamepadCurrentAssociatedKey[key].m_keyID, GLFW_PRESS))
            {
                return true;
            }
        }
        else
        {
            assert(m_mapGamepadCurrentAssociatedKey[key].m_axisPos);
            if(checkAxisGamepadKeyStatus(m_mapGamepadCurrentAssociatedKey[key].m_keyID, *m_mapGamepadCurrentAssociatedKey[key].m_axisPos))
            {
                return true;
            }
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
        updateDetectRect(playerComp, mapComp);
    }
}

//===================================================================
void InputSystem::window_focus_callback(GLFWwindow *window, int focused)
{
    if(focused == 0)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_windowFocus = false;
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_windowFocus = true;
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
    if(m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_A] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_A, GLFW_RELEASE))
    {
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_A] = false;
    }
    if(m_enterPressed || m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_A])
    {
        return;
    }
    if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
    {
        m_keyEspapePressed = false;
    }
    if((!m_keyEspapePressed && glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) ||
            (!m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_B] && playerComp->m_menuMode != MenuMode_e::NEW_KEY
              && playerComp->m_menuMode != MenuMode_e::TITLE &&
             checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_B, GLFW_PRESS)))
    {
        m_keyEspapePressed = true;
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_B] = true;
        if(playerComp->m_menuMode == MenuMode_e::BASE || playerComp->m_menuMode == MenuMode_e::TRANSITION_LEVEL)
        {
            m_mainEngine->setUnsetPaused();
        }
        else if(playerComp->m_menuMode == MenuMode_e::NEW_KEY)
        {
            playerComp->m_menuMode = MenuMode_e::INPUT;
            m_mainEngine->setMenuEntries(playerComp);
        }
        else if(playerComp->m_inputModified && playerComp->m_menuMode == MenuMode_e::INPUT)
        {
            playerComp->m_menuMode = MenuMode_e::CONFIRM_QUIT_INPUT_FORM;
            m_mainEngine->setMenuEntries(playerComp);
        }
        else if(playerComp->m_menuMode == MenuMode_e::CONFIRM_LOADING_GAME_FORM)
        {
            playerComp->m_menuMode = playerComp->m_previousMenuMode;
            m_mainEngine->setMenuEntries(playerComp);
        }
        else if(playerComp->m_menuMode == MenuMode_e::TITLE)
        {
            playerComp->m_currentCursorPos = static_cast<uint32_t>(TitleMenuCursorPos_e::QUIT_GAME);
        }
        else if(playerComp->m_menuMode != MenuMode_e::CONFIRM_QUIT_INPUT_FORM)
        {
            if(playerComp->m_menuMode == MenuMode_e::SOUND)
            {
                m_mainEngine->saveAudioSettings();
            }
            else if(playerComp->m_menuMode == MenuMode_e::INPUT)
            {
                m_mainEngine->saveTurnSensitivitySettings();
            }
            playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
            m_mainEngine->setMenuEntries(playerComp);
        }
        return;
    }
    if(playerComp->m_menuMode == MenuMode_e::NEW_KEY)
    {
        treatAxisRelease();
        if(treatNewKey(playerComp))
        {
            playerComp->m_inputModified = true;
            playerComp->m_menuMode = MenuMode_e::INPUT;
            m_mainEngine->setMenuEntries(playerComp);
            m_gamepadAxisKeyPressed.fill({false, false});
        }
    }
    treatReleaseInputMenu();
    treatGeneralKeysMenu(playerComp);
}

//===================================================================
void InputSystem::treatAxisRelease()
{
    for(uint32_t i = 0; i < m_gamepadAxisKeyPressed.size(); ++i)
    {
        if(!checkAxisGamepadKeyStatus(i, true))
        {
            m_gamepadAxisKeyPressed[i].first = false;
        }
        if(!checkAxisGamepadKeyStatus(i, false))
        {
            m_gamepadAxisKeyPressed[i].second = false;
        }
    }
}

//===================================================================
void InputSystem::treatReleaseInputMenu()
{
    //UP
    if(m_keyUpPressed && glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_RELEASE)
    {
        m_keyUpPressed = false;
    }
    if(m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_UP] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_RELEASE))
    {
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_UP] = false;
    }
    //DOWN
    if(m_keyDownPressed && glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_RELEASE)
    {
        m_keyDownPressed = false;
    }
    if(m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_RELEASE))
    {
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] = false;
    }
    //LEFT
    if(m_keyLeftPressed && glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_RELEASE)
    {
        m_keyLeftPressed = false;
    }
    if(m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_RELEASE))
    {
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] = false;
    }
    //RIGHT
    if(m_keyRightPressed && glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
    {
        m_keyRightPressed = false;
    }
    if(m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_RELEASE))
    {
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = false;
    }
    //TOOGLE GAMEPAD KEYBOARD INPUT MENU
    if(m_keyKeyboardGPressed && glfwGetKey(m_window, GLFW_KEY_G) == GLFW_RELEASE)
    {
        m_keyKeyboardGPressed = false;
    }
    if(m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_RELEASE))
    {
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] = false;
    }
}

//===================================================================
void InputSystem::treatGeneralKeysMenu(PlayerConfComponent *playerComp)
{
    uint32_t maxMenuIndex = m_mapMenuSize.at(playerComp->m_menuMode);
    if(!m_modeTransition && ((!m_keyUpPressed && glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) ||
                             (!m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_UP] &&
                              checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_PRESS))))
    {
        m_keyUpPressed = true;
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_UP] = true;
        decrementMenuPosition(playerComp, maxMenuIndex);
        if(playerComp->m_menuMode == MenuMode_e::BASE &&
                static_cast<MainMenuCursorPos_e>(playerComp->m_currentCursorPos) ==
                MainMenuCursorPos_e::RESTART_FROM_LAST_CHECKPOINT && !m_mainEngine->checkpointActive())
        {
            decrementMenuPosition(playerComp, maxMenuIndex);
        }
    }
    else if(!m_modeTransition && ((!m_keyDownPressed && (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)) ||
                                  (!m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_PRESS))))
    {
        m_keyDownPressed = true;
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] = true;
        incrementMenuPosition(playerComp, maxMenuIndex);
        if(playerComp->m_menuMode == MenuMode_e::BASE &&
                static_cast<MainMenuCursorPos_e>(playerComp->m_currentCursorPos) ==
                MainMenuCursorPos_e::RESTART_FROM_LAST_CHECKPOINT && !m_mainEngine->checkpointActive())
        {
            incrementMenuPosition(playerComp, maxMenuIndex);
        }
    }
    else if((!m_enterPressed && glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_PRESS) ||
            (!m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_A] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_A, GLFW_PRESS)))
    {
        treatEnterPressedMenu(playerComp);
    }
    else if((!m_keyLeftPressed && (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)) ||
             (!m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_PRESS)))
    {
        treatLeftPressedMenu(playerComp);
    }
    else if((!m_keyRightPressed && (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)) ||
             (!m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] && checkStandardButtonGamepadKeyStatus(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_PRESS)))
    {
        treatRightPressedMenu(playerComp);
    }
    else if(!m_keyKeyboardGPressed && !m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] &&
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
    if(!m_mapGamepadID.empty())
    {
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] = true;
    }
    playerComp->m_keyboardInputMenuMode = !playerComp->m_keyboardInputMenuMode;
    m_mainEngine->updateMenuInfo(playerComp);
    mptrSystemManager->searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM))->
            updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode, false);
}

//===================================================================
bool InputSystem::treatNewKey(PlayerConfComponent *playerComp)
{
    StaticDisplaySystem *staticSystem = mptrSystemManager->searchSystemByType<StaticDisplaySystem>(
                static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    if(playerComp->m_keyboardInputMenuMode)
    {
        //KEYBOARD
        for(std::map<uint32_t, std::string>::const_iterator it = INPUT_KEYBOARD_KEY_STRING.begin(); it != INPUT_KEYBOARD_KEY_STRING.end(); ++it)
        {
            if(glfwGetKey(m_window, it->first) == GLFW_PRESS)
            {
                if(it->first == GLFW_KEY_G)
                {
                    m_keyKeyboardGPressed = true;
                }
                else if(it->first == GLFW_KEY_UP)
                {
                    m_keyUpPressed = true;
                }
                else if(it->first == GLFW_KEY_DOWN)
                {
                    m_keyDownPressed = true;
                }
                else if(it->first == GLFW_KEY_ENTER)
                {
                    m_enterPressed = true;
                }
                m_mapKeyboardTmpAssociatedKey[m_currentSelectedKey] = {true, it->first};
                staticSystem->updateNewInputKeyKeyboard(m_currentSelectedKey, m_mapKeyboardTmpAssociatedKey[m_currentSelectedKey]);
                return true;
            }
        }
        int state;
        //MOUSE
        for(std::map<uint32_t, std::string>::const_iterator it = INPUT_MOUSE_KEY_STRING.begin();
            it != INPUT_MOUSE_KEY_STRING.end(); ++it)
        {
            state = glfwGetMouseButton(m_window, it->first);
            if(state == GLFW_PRESS)
            {

                m_mapKeyboardTmpAssociatedKey[m_currentSelectedKey] = {false, it->first};
                staticSystem->updateNewInputKeyKeyboard(m_currentSelectedKey, m_mapKeyboardTmpAssociatedKey[m_currentSelectedKey]);
                return true;
            }
        }
    }
    else
    {
        //BUTTONS
        for(std::map<uint32_t, std::string>::const_iterator it = INPUT_GAMEPAD_SIMPLE_BUTTONS_STRING.begin();
            it != INPUT_GAMEPAD_SIMPLE_BUTTONS_STRING.end(); ++it)
        {
            if(it->first != GLFW_GAMEPAD_BUTTON_START && checkStandardButtonGamepadKeyStatus(it->first, GLFW_PRESS))
            {
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_standardButton = true;
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_axisPos = {};
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_keyID = it->first;
                staticSystem->updateNewInputKeyGamepad(m_currentSelectedKey, it->first, InputType_e::GAMEPAD_BUTTONS);
                if(it->first == GLFW_GAMEPAD_BUTTON_B)
                {
                    m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_B] = true;
                }
                else if(it->first == GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER)
                {
                    m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] = true;
                }
                else if(it->first == GLFW_GAMEPAD_BUTTON_DPAD_UP)
                {
                    m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_UP] = true;
                }
                else if(it->first == GLFW_GAMEPAD_BUTTON_DPAD_DOWN)
                {
                    m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] = true;
                }
                else if(it->first == GLFW_GAMEPAD_BUTTON_A)
                {
                    m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_A] = true;
                }
                return true;
            }
        }
        //AXIS
        for(std::map<uint32_t, std::string>::const_iterator it = INPUT_GAMEPAD_AXIS_STRING.begin(); it != INPUT_GAMEPAD_AXIS_STRING.end(); ++it)
        {
            //POS
            if(!m_gamepadAxisKeyPressed[it->first].first && checkAxisGamepadKeyStatus(it->first, true))
            {
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_standardButton = false;
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_axisPos = true;
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_keyID = it->first;
                staticSystem->updateNewInputKeyGamepad(m_currentSelectedKey, it->first, InputType_e::GAMEPAD_AXIS, true);
                return true;
            }
            //NEG
            else if(!m_gamepadAxisKeyPressed[it->first].second && checkAxisGamepadKeyStatus(it->first, false))
            {
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_standardButton = false;
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_axisPos = false;
                m_mapGamepadTmpAssociatedKey[m_currentSelectedKey].m_keyID = it->first;
                staticSystem->updateNewInputKeyGamepad(m_currentSelectedKey, it->first, InputType_e::GAMEPAD_AXIS, false);
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
    if(!m_mapGamepadID.empty())
    {
        m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_A] = true;
    }
    switch(playerComp->m_menuMode)
    {
    case MenuMode_e::TITLE:
        treatEnterPressedTitleMenu(playerComp);
        break;
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
    case MenuMode_e::NEW_GAME:
        treatEnterPressedNewGameMenu(playerComp);
        break;
    case MenuMode_e::LOAD_GAME:
        treatEnterPressedLoadGameMenu(playerComp);
        break;
    case MenuMode_e::CONFIRM_QUIT_INPUT_FORM:
        treatEnterPressedConfirmInputMenu(playerComp);
        break;
    case MenuMode_e::CONFIRM_LOADING_GAME_FORM:
        treatEnterPressedConfirmLoadGameMenu(playerComp);
        break;
    case MenuMode_e::CONFIRM_RESTART_LEVEL:
        treatEnterPressedConfirmRestartLevelMenu(playerComp);
        break;
    case MenuMode_e::CONFIRM_RESTART_FROM_LAST_CHECKPOINT:
        treatEnterPressedConfirmRestartFromLastCheckpointMenu(playerComp);
        break;
    }
}

//===================================================================
void InputSystem::treatEnterPressedConfirmInputMenu(PlayerConfComponent *playerComp)
{
    ConfirmCursorPos_e menuEntry = static_cast<ConfirmCursorPos_e>(playerComp->m_currentCursorPos);
    if(menuEntry == ConfirmCursorPos_e::TRUE)
    {
        validInputMenu(playerComp);
    }
    else
    {
        playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
    }
}

//===================================================================
void InputSystem::treatEnterPressedConfirmRestartLevelMenu(PlayerConfComponent *playerComp)
{
    ConfirmCursorPos_e menuEntry = static_cast<ConfirmCursorPos_e>(playerComp->m_currentCursorPos);
    if(menuEntry == ConfirmCursorPos_e::TRUE)
    {
        if(m_mainEngine->loadSavedGame(m_mainEngine->getCurrentSaveNum(), LevelState_e::RESTART_LEVEL))
        {
            m_mainEngine->setTransition(true);
        }
    }
    else
    {
        playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
    }
}

//===================================================================
void InputSystem::treatEnterPressedConfirmRestartFromLastCheckpointMenu(PlayerConfComponent *playerComp)
{
    ConfirmCursorPos_e menuEntry = static_cast<ConfirmCursorPos_e>(playerComp->m_currentCursorPos);
    if(menuEntry == ConfirmCursorPos_e::TRUE)
    {
        if(m_mainEngine->loadSavedGame(m_mainEngine->getCurrentSaveNum(), LevelState_e::RESTART_FROM_CHECKPOINT))
        {
            m_mainEngine->setTransition(true);
        }
    }
    else
    {
        playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
    }
}

//===================================================================
void InputSystem::treatEnterPressedConfirmLoadGameMenu(PlayerConfComponent *playerComp)
{
    ConfirmCursorPos_e menuEntry = static_cast<ConfirmCursorPos_e>(playerComp->m_currentCursorPos);
    if(menuEntry == ConfirmCursorPos_e::TRUE)
    {
        //NEW GAME
        if(playerComp->m_previousMenuMode == MenuMode_e::NEW_GAME)
        {
            uint32_t numSaveFile = playerComp->m_currentSelectedSaveFile;
            //init or reinit save file
            m_mainEngine->savePlayerGear(true);
            m_mainEngine->saveGameProgress(1, numSaveFile);
            if(m_mainEngine->loadSavedGame(numSaveFile, LevelState_e::NEW_GAME))
            {
                m_mainEngine->setTransition(true);
            }
        }
        //LOAD
        else
        {
            if(m_mainEngine->loadSavedGame(playerComp->m_currentSelectedSaveFile, LevelState_e::LOAD_GAME))
            {
                m_mainEngine->setTransition(true);
            }
        }
    }
    else
    {
        playerComp->m_menuMode = playerComp->m_previousMenuMode;
        m_mainEngine->setMenuEntries(playerComp);
    }
}

//===================================================================
void InputSystem::treatLeftPressedMenu(PlayerConfComponent *playerComp)
{
    if(playerComp->m_menuMode == MenuMode_e::INPUT)
    {
        InputMenuCursorPos_e inputCursorPos = static_cast<InputMenuCursorPos_e>(playerComp->m_currentCursorPos);
        if(inputCursorPos == InputMenuCursorPos_e::TURN_SENSITIVITY)
        {
            uint32_t turnSensitivity = m_mainEngine->getTurnSensitivity();
            if(turnSensitivity > MIN_TURN_SENSITIVITY)
            {
                m_mainEngine->updateTurnSensitivity(--turnSensitivity);
            }
        }
    }
    else if(playerComp->m_menuMode == MenuMode_e::SOUND)
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
        if(!m_mapGamepadID.empty())
        {
            m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] = true;
        }
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
        else if(displayCursorPos == DisplayMenuCursorPos_e::FULLSCREEN)
        {
            m_mainEngine->toogleMenuEntryFullscreen();
        }
    }
}

//===================================================================
void InputSystem::treatRightPressedMenu(PlayerConfComponent *playerComp)
{
    if(playerComp->m_menuMode == MenuMode_e::INPUT)
    {
        InputMenuCursorPos_e inputCursorPos = static_cast<InputMenuCursorPos_e>(playerComp->m_currentCursorPos);
        if(inputCursorPos == InputMenuCursorPos_e::TURN_SENSITIVITY)
        {
            uint32_t turnSensitivity = m_mainEngine->getTurnSensitivity();
            if(turnSensitivity < MAX_TURN_SENSITIVITY)
            {
                m_mainEngine->updateTurnSensitivity(++turnSensitivity);
            }
        }
    }
    else if(playerComp->m_menuMode == MenuMode_e::SOUND)
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
        if(!m_mapGamepadID.empty())
        {
            m_gamepadButtonsKeyPressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = true;
        }
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
        else if(displayCursorPos == DisplayMenuCursorPos_e::FULLSCREEN)
        {
            m_mainEngine->toogleMenuEntryFullscreen();
        }
    }
}

//===================================================================
void InputSystem::treatEnterPressedTitleMenu(PlayerConfComponent *playerComp)
{
    TitleMenuCursorPos_e menuPos = static_cast<TitleMenuCursorPos_e>(playerComp->m_currentCursorPos);
    switch(menuPos)
    {
    case TitleMenuCursorPos_e::SOUND_CONF:
        playerComp->m_menuMode = MenuMode_e::SOUND;
        m_mainEngine->setMenuEntries(playerComp);
        m_mainEngine->getMusicVolume();
        break;
    case TitleMenuCursorPos_e::DISPLAY_CONF:
        playerComp->m_menuMode = MenuMode_e::DISPLAY;
        m_mainEngine->setMenuEntries(playerComp);
        break;
    case TitleMenuCursorPos_e::INPUT_CONF:
        playerComp->m_inputModified = false;
        playerComp->m_menuMode = MenuMode_e::INPUT;
        m_mainEngine->setMenuEntries(playerComp);
        m_mapKeyboardTmpAssociatedKey = m_mapKeyboardCurrentAssociatedKey;
        m_mapGamepadTmpAssociatedKey = m_mapGamepadCurrentAssociatedKey;
        m_mainEngine->updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode, false);
        break;
    case TitleMenuCursorPos_e::NEW_GAME:
        playerComp->m_menuMode = MenuMode_e::NEW_GAME;
        m_mainEngine->setMenuEntries(playerComp);
        break;
    case TitleMenuCursorPos_e::LOAD_GAME:
        playerComp->m_menuMode = MenuMode_e::LOAD_GAME;
        m_mainEngine->setMenuEntries(playerComp);
        break;
    case TitleMenuCursorPos_e::QUIT_GAME:
        glfwSetWindowShouldClose(m_window, true);
        break;
    case TitleMenuCursorPos_e::PLAY_CUSTOM_LEVELS:
    case TitleMenuCursorPos_e::TOTAL:
        break;
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
        playerComp->m_inputModified = false;
        playerComp->m_menuMode = MenuMode_e::INPUT;
        m_mainEngine->setMenuEntries(playerComp);
        m_mapKeyboardTmpAssociatedKey = m_mapKeyboardCurrentAssociatedKey;
        m_mapGamepadTmpAssociatedKey = m_mapGamepadCurrentAssociatedKey;
        m_mainEngine->updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode, false);
        break;
    case MainMenuCursorPos_e::NEW_GAME:
        playerComp->m_menuMode = MenuMode_e::NEW_GAME;
        m_mainEngine->setMenuEntries(playerComp);
        break;
    case MainMenuCursorPos_e::LOAD_GAME:
        playerComp->m_menuMode = MenuMode_e::LOAD_GAME;
        m_mainEngine->setMenuEntries(playerComp);
        break;
    case MainMenuCursorPos_e::RESTART_FROM_LAST_CHECKPOINT:
        playerComp->m_menuMode = MenuMode_e::CONFIRM_RESTART_FROM_LAST_CHECKPOINT;
        m_mainEngine->setMenuEntries(playerComp);
        break;
    case MainMenuCursorPos_e::RESTART_LEVEL:
        playerComp->m_menuMode = MenuMode_e::CONFIRM_RESTART_LEVEL;
        m_mainEngine->setMenuEntries(playerComp);
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
        m_mainEngine->saveAudioSettings();
        playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
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
        playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
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
    if(menuPos == InputMenuCursorPos_e::RETURN)
    {
        if(playerComp->m_inputModified)
        {
            playerComp->m_menuMode = MenuMode_e::CONFIRM_QUIT_INPUT_FORM;
            m_mainEngine->setMenuEntries(playerComp);
        }
        else
        {
            playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
            m_mainEngine->setMenuEntries(playerComp);
            m_mainEngine->saveTurnSensitivitySettings();
        }
    }
    else if(menuPos == InputMenuCursorPos_e::DEFAULT)
    {
        playerComp->m_inputModified = true;
        if(playerComp->m_keyboardInputMenuMode)
        {
            m_mapKeyboardTmpAssociatedKey = m_mapKeyboardCurrentAssociatedKey;
            m_mapKeyboardCurrentAssociatedKey = MAP_KEYBOARD_DEFAULT_KEY;
            m_mainEngine->updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode);
            m_mapKeyboardCurrentAssociatedKey = m_mapKeyboardTmpAssociatedKey;
            m_mapKeyboardTmpAssociatedKey = MAP_KEYBOARD_DEFAULT_KEY;
        }
        else
        {
            m_mapGamepadTmpAssociatedKey = m_mapGamepadCurrentAssociatedKey;
            m_mapGamepadCurrentAssociatedKey = MAP_GAMEPAD_DEFAULT_KEY;
            m_mainEngine->updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode);
            m_mapGamepadCurrentAssociatedKey = m_mapGamepadTmpAssociatedKey;
            m_mapGamepadTmpAssociatedKey = MAP_GAMEPAD_DEFAULT_KEY;
        }
        m_mainEngine->updateTurnSensitivity(40);
    }
    else if(menuPos == InputMenuCursorPos_e::VALID)
    {
        validInputMenu(playerComp);
    }
    //INPUT CHANGE
    else if(menuPos != InputMenuCursorPos_e::TURN_SENSITIVITY)
    {
        m_gamepadAxisKeyPressed.fill({true, true});
        playerComp->m_menuMode = MenuMode_e::NEW_KEY;
        m_mainEngine->setMenuEntries(playerComp);
        m_currentSelectedKey = m_mapInputControl.at(menuPos);
    }
}

//===================================================================
void InputSystem::treatEnterPressedNewGameMenu(PlayerConfComponent *playerComp)
{
    NewGameMenuCursorPos_e menuPos = static_cast<NewGameMenuCursorPos_e>(playerComp->m_currentCursorPos);
    if(menuPos == NewGameMenuCursorPos_e::RETURN)
    {
        playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
    }
    else
    {
        playerComp->m_previousMenuMode = playerComp->m_menuMode;
        playerComp->m_currentSelectedSaveFile = playerComp->m_currentCursorPos + 1;
        m_mainEngine->updateMenuInfo(playerComp);
        playerComp->m_menuMode = MenuMode_e::CONFIRM_LOADING_GAME_FORM;
        m_mainEngine->setMenuEntries(playerComp);
    }
}

//===================================================================
void InputSystem::treatEnterPressedLoadGameMenu(PlayerConfComponent *playerComp)
{
    LoadGameMenuCursorPos_e menuPos = static_cast<LoadGameMenuCursorPos_e>(playerComp->m_currentCursorPos);
    if(menuPos == LoadGameMenuCursorPos_e::RETURN)
    {
        playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
        m_mainEngine->setMenuEntries(playerComp);
    }
    else
    {
        if(!m_mainEngine->checkSavedGameExists(playerComp->m_currentCursorPos + 1))
        {
            return;
        }
        playerComp->m_currentSelectedSaveFile = playerComp->m_currentCursorPos + 1;
        playerComp->m_previousMenuMode = playerComp->m_menuMode;
        m_mainEngine->updateMenuInfo(playerComp);
        playerComp->m_menuMode = MenuMode_e::CONFIRM_LOADING_GAME_FORM;
        m_mainEngine->setMenuEntries(playerComp);
    }
}

//===================================================================
void InputSystem::validInputMenu(PlayerConfComponent *playerComp)
{
    if(playerComp->m_keyboardInputMenuMode)
    {
        m_mapKeyboardCurrentAssociatedKey = m_mapKeyboardTmpAssociatedKey;
    }
    else
    {
        m_mapGamepadCurrentAssociatedKey = m_mapGamepadTmpAssociatedKey;
    }
    m_mainEngine->saveInputSettings(m_mapGamepadCurrentAssociatedKey, m_mapKeyboardCurrentAssociatedKey);
    m_mainEngine->saveTurnSensitivitySettings();
    m_mainEngine->updateStringWriteEntitiesInputMenu(playerComp->m_keyboardInputMenuMode);
    playerComp->m_menuMode = playerComp->m_firstMenu ? MenuMode_e::TITLE : MenuMode_e::BASE;
    m_mainEngine->setMenuEntries(playerComp);
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

//===================================================================
void InputSystem::updateNewInputKeyGamepad(ControlKey_e currentSelectedKey, uint32_t glKey, InputType_e inputType, bool axisSense)
{
    if(inputType == InputType_e::GAMEPAD_BUTTONS)
    {
        if(glKey == GLFW_GAMEPAD_BUTTON_START)
        {
            return;
        }
        m_mapGamepadCurrentAssociatedKey[currentSelectedKey].m_standardButton = true;
    }
    else
    {
        m_mapGamepadCurrentAssociatedKey[currentSelectedKey].m_standardButton = false;
        m_mapGamepadCurrentAssociatedKey[currentSelectedKey].m_axisPos = axisSense;
    }
    m_mapGamepadCurrentAssociatedKey[currentSelectedKey].m_keyID = glKey;
}

//===================================================================
void InputSystem::updateNewInputKeyKeyboard(ControlKey_e currentSelectedKey, const MouseKeyboardInputState &state)
{
    m_mapKeyboardCurrentAssociatedKey[currentSelectedKey] = state;
}

//===================================================================
void InputSystem::addGamepad(int gamepadID)
{
    if(m_mapGamepadID.find(gamepadID) == m_mapGamepadID.end())
    {
        m_mapGamepadID.insert({gamepadID, {nullptr, nullptr}});
    }
}

//===================================================================
void InputSystem::scroll_callback(GLFWwindow *window, double xOffset, double yOffset)
{
    if(std::abs(yOffset) < 0.01)
    {
        return;
    }
    if(yOffset < 0.0)
    {
        m_scrollDown = true;
    }
    else
    {
        m_scrollUp = true;
    }
}

//===================================================================
void InputSystem::updateDetectRect(PlayerConfComponent *playerComp, MapCoordComponent *mapPlayerComp)
{
    MapCoordComponent *mapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(playerComp->m_mapDetectShapeEntity, Components_e::MAP_COORD_COMPONENT);
    assert(mapComp);
    mapComp->m_absoluteMapPositionPX = {mapPlayerComp->m_absoluteMapPositionPX.first - DETECT_RECT_SHAPE_HALF_SIZE,
                                       mapPlayerComp->m_absoluteMapPositionPX.second - DETECT_RECT_SHAPE_HALF_SIZE};
}

//===================================================================
void InputSystem::removeGamepad(int gamepadID)
{
    MapGamepadInputData_t::iterator it = m_mapGamepadID.find(gamepadID);
    if(it != m_mapGamepadID.end())
    {
        m_mapGamepadID.erase(it);
    }
}

//===================================================================
void decrementMenuPosition(PlayerConfComponent *playerConf, uint32_t maxIndex)
{
    uint32_t index = playerConf->m_currentCursorPos;
    if(index == 0)
    {
        playerConf->m_currentCursorPos = maxIndex;
    }
    else
    {
        playerConf->m_currentCursorPos = index - 1;
    }
}

//===================================================================
void incrementMenuPosition(PlayerConfComponent *playerConf, uint32_t maxIndex)
{
    uint32_t index = playerConf->m_currentCursorPos;
    if(index == maxIndex)
    {
        playerConf->m_currentCursorPos = 0;
    }
    else
    {
        playerConf->m_currentCursorPos = index + 1;
    }
}

//===================================================================
void joystick_callback(int jid, int event)
{
    if(event == GLFW_CONNECTED)
    {
        InputSystem::addGamepad(jid);
    }
    else if(event == GLFW_DISCONNECTED)
    {
        InputSystem::removeGamepad(jid);
    }
}
