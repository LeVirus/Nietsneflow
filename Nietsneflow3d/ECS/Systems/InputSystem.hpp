#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <constants.hpp>

struct PlayerConfComponent;
struct MoveableComponent;
struct MapCoordComponent;
class MainEngine;
struct WeaponComponent;

class InputSystem : public ecs::System
{
public:
    InputSystem();
    void execSystem()override;
    inline void linkMainEngine(MainEngine *mainEngine)
    {
        m_mainEngine = mainEngine;
    }
    inline void setGLWindow(GLFWwindow &window)
    {
        m_window = &window;
    }
    inline void setModeTransitionMenu(bool transition)
    {
        m_modeTransition = transition;
    }
    inline void reinitToggleFullScreen()
    {
        m_toggleSignal = false;
    }
    inline bool toggleFullScreenSignal()const
    {
        return m_toggleSignal;
    }
    inline const std::map<ControlKey_e, uint32_t> &getMapCurrentDefaultAssociatedKey()const
    {
        return m_mapKeyboardCurrentAssociatedKey;
    }
private:
    void gamepadInit();
    bool checkGamepadKeyStatus(uint32_t key, uint32_t status);
    void setUsedComponents();
    void getGamepadInputs();
    void treatPlayerInput();
    void treatMenu(uint32_t playerEntity);
    void treatReleaseDirectionalInputMenu();
    void treatGeneralKeysMenu(PlayerConfComponent *playerComp);
    bool treatNewKey();
    void treatEnterPressedMenu(PlayerConfComponent *playerComp);
    void treatLeftPressedMenu(PlayerConfComponent *playerComp);
    void treatRightPressedMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedMainMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedSoundMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedDisplayMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedInputMenu(PlayerConfComponent *playerComp);
    void treatPlayerMove(PlayerConfComponent *playerComp, MoveableComponent *moveComp, MapCoordComponent *mapComp);
private:
    std::vector<std::pair<uint32_t, const uint8_t*>> m_vectGamepadID;
    GLFWwindow *m_window = nullptr;
    MainEngine *m_mainEngine = nullptr;
    bool m_keyEspapePressed = false, m_keyLeftPressed = false, m_keyRightPressed = false,
    m_keyUpPressed = false, m_keyDownPressed = false, m_F12Pressed = false, m_enterPressed = false,
    m_keyGamepadButtonBPressed = false, m_keyGamepadLeftPressed = false, m_keyGamepadRightPressed = false,
        m_keyGamepadUpPressed = false, m_keyGamepadDownPressed = false, m_keyGamepadButtonAPressed = false;
    const std::map<MenuMode_e, uint32_t> m_mapMenuSize = {
        {MenuMode_e::BASE, static_cast<uint32_t>(MainMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::DISPLAY, static_cast<uint32_t>(DisplayMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::INPUT, static_cast<uint32_t>(InputMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::TRANSITION_LEVEL, 1},
        {MenuMode_e::SOUND, static_cast<uint32_t>(SoundMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::NEW_KEY, 0}
    };
    const std::map<ControlKey_e, uint32_t> m_mapKeyboardDefaultAssociatedKey = {
        {ControlKey_e::MOVE_FORWARD, GLFW_KEY_UP},
        {ControlKey_e::MOVE_BACKWARD, GLFW_KEY_DOWN},
        {ControlKey_e::STRAFE_LEFT, GLFW_KEY_Q},
        {ControlKey_e::STRAFE_RIGHT, GLFW_KEY_W},
        {ControlKey_e::TURN_LEFT, GLFW_KEY_LEFT},
        {ControlKey_e::TURN_RIGHT, GLFW_KEY_RIGHT},
        {ControlKey_e::ACTION, GLFW_KEY_SPACE},
        {ControlKey_e::SHOOT, GLFW_KEY_LEFT_SHIFT},
        {ControlKey_e::PREVIOUS_WEAPON, GLFW_KEY_E},
        {ControlKey_e::NEXT_WEAPON, GLFW_KEY_R}
    },
    m_mapGamepadDefaultAssociatedKey= {
        {ControlKey_e::MOVE_FORWARD, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB},
        {ControlKey_e::MOVE_BACKWARD, GLFW_KEY_DOWN},
        {ControlKey_e::STRAFE_LEFT, GLFW_KEY_Q},
        {ControlKey_e::STRAFE_RIGHT, GLFW_KEY_W},
        {ControlKey_e::TURN_LEFT, GLFW_KEY_LEFT},
        {ControlKey_e::TURN_RIGHT, GLFW_KEY_RIGHT},
        {ControlKey_e::ACTION, GLFW_GAMEPAD_BUTTON_A},
        {ControlKey_e::SHOOT, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB},
        {ControlKey_e::PREVIOUS_WEAPON, GLFW_GAMEPAD_BUTTON_Y},
        {ControlKey_e::NEXT_WEAPON, GLFW_GAMEPAD_BUTTON_B}
    };
    std::map<ControlKey_e, uint32_t> m_mapKeyboardCurrentAssociatedKey = m_mapKeyboardDefaultAssociatedKey,
    m_mapKeyboardTmpAssociatedKey = m_mapKeyboardCurrentAssociatedKey,
    m_mapGamepadCurrentAssociatedKey = m_mapGamepadDefaultAssociatedKey,
        m_mapGamepadTmpAssociatedKey = m_mapGamepadCurrentAssociatedKey;
    const std::map<InputMenuCursorPos_e, ControlKey_e> m_mapInputControl = {
        {InputMenuCursorPos_e::ACTION, ControlKey_e::ACTION},
        {InputMenuCursorPos_e::MOVE_BACKWARD, ControlKey_e::MOVE_BACKWARD},
        {InputMenuCursorPos_e::MOVE_FORWARD, ControlKey_e::MOVE_FORWARD},
        {InputMenuCursorPos_e::NEXT_WEAPON, ControlKey_e::NEXT_WEAPON},
        {InputMenuCursorPos_e::PREVIOUS_WEAPON, ControlKey_e::PREVIOUS_WEAPON},
        {InputMenuCursorPos_e::SHOOT, ControlKey_e::SHOOT},
        {InputMenuCursorPos_e::STRAFE_LEFT, ControlKey_e::STRAFE_LEFT},
        {InputMenuCursorPos_e::STRAFE_RIGHT, ControlKey_e::STRAFE_RIGHT},
        {InputMenuCursorPos_e::TURN_LEFT, ControlKey_e::TURN_LEFT},
        {InputMenuCursorPos_e::TURN_RIGHT, ControlKey_e::TURN_RIGHT}
    };
    ControlKey_e m_currentSelectedKey;
    bool m_modeTransition = false, m_toggleSignal = false;
};

void changePlayerWeapon(WeaponComponent &weaponComp, bool next);
void changeToTopPlayerWeapon(WeaponComponent &weaponComp);
void setPlayerWeapon(WeaponComponent &weaponComp, uint32_t weapon);
