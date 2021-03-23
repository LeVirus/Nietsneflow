#include "StaticDisplaySystem.hpp"

//===================================================================
StaticDisplaySystem::StaticDisplaySystem()
{
    bAddComponentToSystem(Components_e::VISION_COMPONENT);
}

//===================================================================
void StaticDisplaySystem::execSystem()
{
    System::execSystem();
}

//===================================================================
void StaticDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}
