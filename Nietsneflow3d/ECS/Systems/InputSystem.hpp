#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <constants.hpp>

struct PlayerConfComponent;
struct MoveableComponent;
struct MapCoordComponent;
class MainEngine;
struct WeaponComponent;
enum class InputType_e;

//pair const uint8_t* :: buttons, const float* :: axis
using MapGamepadInputData_t = std::map<uint32_t, std::pair<const uint8_t*, const float*>>;

struct GamepadInputState
{
    //true == standard button, false == axis
    bool m_standardButton;
    uint32_t m_keyID;
    //if m_standardButton == false, indicate the sense
    std::optional<bool> m_axisPos;
};

inline const std::map<ControlKey_e, uint32_t> MAP_KEYBOARD_DEFAULT_KEY = {
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
};

//inline const std::map<ControlKey_e, GamepadInputState> MAP_GAMEPAD_DEFAULT_KEY = {
//    {ControlKey_e::MOVE_FORWARD, GamepadInputState{false, GLFW_GAMEPAD_AXIS_LEFT_Y, false}},
//    {ControlKey_e::MOVE_BACKWARD, GamepadInputState{false, GLFW_GAMEPAD_AXIS_LEFT_Y, true}},
//    {ControlKey_e::STRAFE_LEFT, GamepadInputState{false, GLFW_GAMEPAD_AXIS_LEFT_X, false}},
//    {ControlKey_e::STRAFE_RIGHT, GamepadInputState{false, GLFW_GAMEPAD_AXIS_LEFT_X, true}},
//    {ControlKey_e::TURN_LEFT, GamepadInputState{false, GLFW_GAMEPAD_AXIS_RIGHT_Y, false}},
//    {ControlKey_e::TURN_RIGHT, GamepadInputState{false, GLFW_GAMEPAD_AXIS_RIGHT_Y, true}},
//    {ControlKey_e::ACTION, GamepadInputState{true, GLFW_GAMEPAD_BUTTON_A, {}}},
//    {ControlKey_e::SHOOT, GamepadInputState{true, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, {}}},
//    {ControlKey_e::PREVIOUS_WEAPON, GamepadInputState{true, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, {}}},
//    {ControlKey_e::NEXT_WEAPON, GamepadInputState{true, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, {}}}
//};

inline const std::map<ControlKey_e, GamepadInputState> MAP_GAMEPAD_DEFAULT_KEY= {
    {ControlKey_e::MOVE_FORWARD, GamepadInputState{false, GLFW_GAMEPAD_AXIS_LEFT_Y, true}},
    {ControlKey_e::MOVE_BACKWARD, GamepadInputState{false, GLFW_GAMEPAD_AXIS_LEFT_Y, false}},
    {ControlKey_e::STRAFE_LEFT, GamepadInputState{false, GLFW_GAMEPAD_AXIS_LEFT_X, false}},
    {ControlKey_e::STRAFE_RIGHT, GamepadInputState{false, GLFW_GAMEPAD_AXIS_LEFT_X, true}},
    {ControlKey_e::TURN_LEFT, GamepadInputState{false, GLFW_GAMEPAD_AXIS_RIGHT_X, false}},
    {ControlKey_e::TURN_RIGHT, GamepadInputState{false, GLFW_GAMEPAD_AXIS_RIGHT_X, true}},
    {ControlKey_e::ACTION, GamepadInputState{true, GLFW_GAMEPAD_BUTTON_A, {}}},
    {ControlKey_e::SHOOT, GamepadInputState{true, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, {}}},
    {ControlKey_e::PREVIOUS_WEAPON, GamepadInputState{true, GLFW_GAMEPAD_BUTTON_Y, {}}},
    {ControlKey_e::NEXT_WEAPON, GamepadInputState{true, GLFW_GAMEPAD_BUTTON_B, {}}}
};

//!!!WARNING!!!
// GLFW_GAMEPAD_AXIS_LEFT_X --> OK
// GLFW_GAMEPAD_AXIS_LEFT_Y --> POS & NEG REVERSED
// GLFW_GAMEPAD_AXIS_LEFT_TRIGGER --> GLFW_GAMEPAD_AXIS_RIGHT_Y
// GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER --> OK
// GLFW_GAMEPAD_AXIS_RIGHT_X --> GLFW_GAMEPAD_AXIS_LEFT_TRIGGER
// GLFW_GAMEPAD_AXIS_RIGHT_Y --> GLFW_GAMEPAD_AXIS_RIGHT_X

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
    inline const std::map<ControlKey_e, uint32_t> &getMapTmpKeyboardAssociatedKey()const
    {
        return m_mapKeyboardTmpAssociatedKey;
    }
    inline const std::map<ControlKey_e, GamepadInputState> &getMapTmpGamepadAssociatedKey()const
    {
        return m_mapGamepadTmpAssociatedKey;
    }
    void updateNewInputKey(ControlKey_e currentSelectedKey, uint32_t glKey, InputType_e inputType, bool axisSense = false);
private:
    void gamepadInit();
    bool checkStandardButtonGamepadKeyStatus(uint32_t key, uint32_t status);
    bool checkAxisGamepadKeyStatus(uint32_t key, bool positive);
    void setUsedComponents();
    void getGamepadInputs();
    void treatPlayerInput();
    bool checkPlayerKeyTriggered(ControlKey_e key);
    void treatMenu(uint32_t playerEntity);
    void treatAxisRelease();
    void treatReleaseInputMenu();
    void treatGeneralKeysMenu(PlayerConfComponent *playerComp);
    void toogleInputMenuGamepadKeyboard(PlayerConfComponent *playerComp);
    bool treatNewKey(PlayerConfComponent *playerComp);
    void treatEnterPressedMenu(PlayerConfComponent *playerComp);
    void treatLeftPressedMenu(PlayerConfComponent *playerComp);
    void treatRightPressedMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedMainMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedSoundMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedDisplayMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedInputMenu(PlayerConfComponent *playerComp);
    void validInputMenu(PlayerConfComponent *playerComp);
    void treatPlayerMove(PlayerConfComponent *playerComp, MoveableComponent *moveComp, MapCoordComponent *mapComp);
private:
    MapGamepadInputData_t m_vectGamepadID;
    GLFWwindow *m_window = nullptr;
    MainEngine *m_mainEngine = nullptr;
    bool m_keyEspapePressed = false, m_keyLeftPressed = false, m_keyRightPressed = false,
    m_keyUpPressed = false, m_keyDownPressed = false, m_F12Pressed = false, m_enterPressed = false,
    m_keyKeyboardGPressed = false;
    std::array<bool, GLFW_GAMEPAD_BUTTON_LAST + 1> m_gamepadButtonsKeyPressed;
    //first == axis + second == axis -
    std::array<std::pair<bool, bool>, GLFW_GAMEPAD_AXIS_LAST + 1> m_gamepadAxisKeyPressed;
    const std::map<MenuMode_e, uint32_t> m_mapMenuSize = {
        {MenuMode_e::BASE, static_cast<uint32_t>(MainMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::DISPLAY, static_cast<uint32_t>(DisplayMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::INPUT, static_cast<uint32_t>(InputMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::TRANSITION_LEVEL, 1},
        {MenuMode_e::SOUND, static_cast<uint32_t>(SoundMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::CONFIRM_QUIT_INPUT_FORM, static_cast<uint32_t>(ConfirmQuitInputCursorPos_e::TOTAL) - 1},
        {MenuMode_e::NEW_KEY, 0}
    };
    std::map<ControlKey_e, uint32_t> m_mapKeyboardCurrentAssociatedKey = MAP_KEYBOARD_DEFAULT_KEY,
    m_mapKeyboardTmpAssociatedKey = m_mapKeyboardCurrentAssociatedKey;
    std::map<ControlKey_e, GamepadInputState> m_mapGamepadCurrentAssociatedKey = MAP_GAMEPAD_DEFAULT_KEY,
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
