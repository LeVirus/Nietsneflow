#pragma once

#include <OpenGLUtils/Shader.hpp>
#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"

/**
 * @brief The StaticDisplaySystem class displays the menu
 * and the weapon in use.
 */
class StaticDisplaySystem : public ecs::System
{
public:
    StaticDisplaySystem();
    void execSystem()override;
    void setShader(Shader &shader);
private:
    Shader *m_shader;
};
