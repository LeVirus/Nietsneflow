#include "ColorDisplaySystem.hpp"
#include <constants.hpp>
#include <includesLib/BaseECS/engine.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>

//===================================================================
ColorDisplaySystem::ColorDisplaySystem()
{
}

//===================================================================
void ColorDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::COLOR_VERTEX_COMPONENT);
    bAddComponentToSystem(Components_e::POSITION_VERTEX_COMPONENT);
    bAddExcludeComponentToSystem(Components_e::SPRITE_TEXTURE_COMPONENT);
}

//===================================================================
void ColorDisplaySystem::execSystem()
{
    System::execSystem();

}

//===================================================================
void ColorDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

