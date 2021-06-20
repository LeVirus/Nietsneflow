#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <constants.hpp>

struct PlayerConfComponent;
struct MoveableComponent;
struct MapCoordComponent;
class MainEngine;

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
private:
    void setUsedComponents();
    void treatPlayerInput();
    void treatMainMenu(uint32_t playerEntity);
    void treatPlayerMove(PlayerConfComponent *playerComp, MoveableComponent *moveComp,
                         MapCoordComponent *mapComp);
private:
    GLFWwindow *m_window = nullptr;
    MainEngine *m_mainEngine = nullptr;
    bool m_keyEspapePressed = false, m_keyUpPressed = false, m_keyDownPressed = false;
    uint32_t m_maxMenuCursorIndex = static_cast<uint32_t>(CurrentMenuCursorPos_e::TOTAL) - 1;
    bool m_modeTransition = false;
};

void changePlayerWeapon(PlayerConfComponent &playerComp, bool next);
void setPlayerWeapon(PlayerConfComponent &playerComp, WeaponsType_e weapon);
