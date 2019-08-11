#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/glheaders.hpp>

class InputSystem : public ecs::System
{
private:
    GLFWwindow *m_window = nullptr;
private:
    void setUsedComponents();
    void treatPlayerInput();

public:
    InputSystem();
    void execSystem()override;
    inline void setGLWindow(GLFWwindow &window)
    {
        m_window = &window;
    }
};

#endif // INPUTSYSTEM_H
