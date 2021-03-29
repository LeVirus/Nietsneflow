#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/glheaders.hpp>

struct PlayerConfComponent;

class InputSystem : public ecs::System
{
public:
    InputSystem();
    void execSystem()override;
    inline void setGLWindow(GLFWwindow &window)
    {
        m_window = &window;
    }
private:
    void setUsedComponents();
    void treatPlayerInput();
private:
    GLFWwindow *m_window = nullptr;
};
void changePlayerWeapon(PlayerConfComponent &playerComp, bool next);
