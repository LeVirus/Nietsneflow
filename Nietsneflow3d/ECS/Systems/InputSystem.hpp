#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/glheaders.hpp>

struct PlayerConfComponent;
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
private:
    void setUsedComponents();
    void treatPlayerInput();
private:
    GLFWwindow *m_window = nullptr;
    MainEngine *m_mainEngine = nullptr;
};
void changePlayerWeapon(PlayerConfComponent &playerComp, bool next);
