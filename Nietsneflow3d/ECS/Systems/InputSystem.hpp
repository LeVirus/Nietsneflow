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
private:
    void setUsedComponents();
    void treatPlayerInput();
    void treatMenu(uint32_t playerEntity);
    void treatGeneralKeysMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedMenu(PlayerConfComponent *playerComp);
    void treatLeftPressedMenu(PlayerConfComponent *playerComp);
    void treatRightPressedMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedMainMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedSoundMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedDisplayMenu(PlayerConfComponent *playerComp);
    void treatEnterPressedInputMenu(PlayerConfComponent *playerComp);
    void treatPlayerMove(PlayerConfComponent *playerComp, MoveableComponent *moveComp, MapCoordComponent *mapComp);
private:
    GLFWwindow *m_window = nullptr;
    MainEngine *m_mainEngine = nullptr;
    bool m_keyEspapePressed = false, m_keyLeftPressed = false, m_keyRightPressed = false,
    m_keyUpPressed = false, m_keyDownPressed = false, m_F12Pressed = false, m_enterPressed = false;
    const std::map<MenuMode_e, uint32_t> m_mapMenuSize = {
        {MenuMode_e::BASE, static_cast<uint32_t>(MainMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::DISPLAY, static_cast<uint32_t>(DisplayMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::INPUT, static_cast<uint32_t>(InputMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::TRANSITION_LEVEL, 1},
        {MenuMode_e::SOUND, static_cast<uint32_t>(SoundMenuCursorPos_e::TOTAL) - 1},
        {MenuMode_e::NEW_KEY, 0}
    };
    const std::map<ControlKey_e, uint32_t> m_mapDefaultAssociatedKey = {
        {ControlKey_e::ACTION, GLFW_KEY_SPACE},
        {ControlKey_e::MOVE_FORWARD, GLFW_KEY_UP},
        {ControlKey_e::MOVE_BACKWARD, GLFW_KEY_DOWN},
        {ControlKey_e::SHOOT, GLFW_KEY_LEFT_SHIFT},
        {ControlKey_e::STRAFE_LEFT, GLFW_KEY_Q},
        {ControlKey_e::STRAFE_RIGHT, GLFW_KEY_W},
        {ControlKey_e::TURN_LEFT, GLFW_KEY_LEFT},
        {ControlKey_e::TURN_RIGHT, GLFW_KEY_RIGHT},
        {ControlKey_e::PREVIOUS_WEAPON, GLFW_KEY_E},
        {ControlKey_e::NEXT_WEAPON, GLFW_KEY_R}
    };
    std::map<ControlKey_e, uint32_t> m_mapCurrentAssociatedKey = m_mapDefaultAssociatedKey;
    bool m_modeTransition = false, m_toggleSignal = false;
};

void changePlayerWeapon(WeaponComponent &weaponComp, bool next);
void changeToTopPlayerWeapon(WeaponComponent &weaponComp);
void setPlayerWeapon(WeaponComponent &weaponComp, uint32_t weapon);
