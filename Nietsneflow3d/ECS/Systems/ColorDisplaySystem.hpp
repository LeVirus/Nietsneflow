#ifndef COLORDISPLAYSYSTEM_H
#define COLORDISPLAYSYSTEM_H

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>

class ColorDisplaySystem : public ecs::System
{
private:
    Shader *m_shader;
public:
    ColorDisplaySystem();
    void setUsedComponents();
    void execSystem()override;
    void setShader(Shader &shader);
};

#endif // COLORDISPLAYSYSTEM_H
