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
    inline bool toggleFullScreen()const
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
    bool m_keyEspapePressed = false, m_keyUpPressed = false, m_keyDownPressed = false, m_F12Pressed = false, m_enterPressed = false;
    const std::map<MenuMode_e, uint32_t> m_mapMenuSize = {{MenuMode_e::BASE, static_cast<uint32_t>(MainMenuCursorPos_e::TOTAL) - 1},
                                                          {MenuMode_e::DISPLAY, static_cast<uint32_t>(DisplayMenuCursorPos_e::TOTAL) - 1},
                                                          {MenuMode_e::INPUT, static_cast<uint32_t>(InputMenuCursorPos_e::TOTAL) - 1},
                                                          {MenuMode_e::TRANSITION_LEVEL, 1},
                                                          {MenuMode_e::SOUND, static_cast<uint32_t>(SoundMenuCursorPos_e::TOTAL) - 1}};
    bool m_modeTransition = false, m_toggleSignal = false;
};

void changePlayerWeapon(WeaponComponent &weaponComp, bool next);
void changeToTopPlayerWeapon(WeaponComponent &weaponComp);
void setPlayerWeapon(WeaponComponent &weaponComp, uint32_t weapon);
