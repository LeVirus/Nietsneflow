#include "ColorDisplaySystem.hpp"
#include <constants.hpp>
#include <includesLib/BaseECS/engine.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <cassert>

//===================================================================
ColorDisplaySystem::ColorDisplaySystem() : m_verticesData(Shader_e::COLOR_S)
{
    setUsedComponents();
}

//===================================================================
void ColorDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::COLOR_VERTEX_COMPONENT);
    bAddComponentToSystem(Components_e::POSITION_VERTEX_COMPONENT);
    bAddExcludeComponentToSystem(Components_e::SPRITE_TEXTURE_COMPONENT);
    bAddExcludeComponentToSystem(Components_e::MAP_COORD_COMPONENT);
}

//===================================================================
void ColorDisplaySystem::fillVertexFromEntities()
{
    m_verticesData.clear();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(mVectNumEntity[i],
                                                               Components_e::POSITION_VERTEX_COMPONENT);
        ColorVertexComponent *colorComp = stairwayToComponentManager().
                searchComponentByType<ColorVertexComponent>(mVectNumEntity[i],
                                                            Components_e::COLOR_VERTEX_COMPONENT);
        assert(posComp);
        assert(colorComp);
        m_verticesData.loadVertexColorComponent(posComp, colorComp);
    }
}

//===================================================================
void ColorDisplaySystem::drawVertex()
{
    m_verticesData.confVertexBuffer();
    m_shader->use();
    m_verticesData.drawElement();
}

//===================================================================
void ColorDisplaySystem::execSystem()
{
    fillVertexFromEntities();
    drawVertex();
}

//===================================================================
void ColorDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

//===================================================================
void ColorDisplaySystem::addColorSystemEntity(uint32_t entity)
{
    mVectNumEntity.emplace_back(entity);
}

//===================================================================
void ColorDisplaySystem::loadColorEntities(uint32_t damage, uint32_t getObject, uint32_t transition)
{
    m_damageMemComponents.first = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(damage, Components_e::POSITION_VERTEX_COMPONENT);
    m_damageMemComponents.second = stairwayToComponentManager().
            searchComponentByType<ColorVertexComponent>(damage, Components_e::COLOR_VERTEX_COMPONENT);
    assert(m_damageMemComponents.first);
    assert(m_damageMemComponents.second);

    m_getObjectMemComponents.first = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(getObject, Components_e::POSITION_VERTEX_COMPONENT);
    m_getObjectMemComponents.second = stairwayToComponentManager().
            searchComponentByType<ColorVertexComponent>(getObject, Components_e::COLOR_VERTEX_COMPONENT);
    assert(m_getObjectMemComponents.first);
    assert(m_getObjectMemComponents.second);

    m_transitionMemComponents.first = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(transition, Components_e::POSITION_VERTEX_COMPONENT);
    m_transitionMemComponents.second = stairwayToComponentManager().
            searchComponentByType<ColorVertexComponent>(transition, Components_e::COLOR_VERTEX_COMPONENT);
    assert(m_transitionMemComponents.first);
    assert(m_transitionMemComponents.second);
}

//===================================================================
void ColorDisplaySystem::drawEntity(const PositionVertexComponent *posComp, const ColorVertexComponent *colorComp)
{
    m_verticesData.clear();
    m_verticesData.loadVertexColorComponent(posComp, colorComp);
    drawVertex();
}

//===================================================================
void ColorDisplaySystem::drawVisibleDamage()
{
    drawEntity(m_damageMemComponents.first, m_damageMemComponents.second);
}

void ColorDisplaySystem::drawVisiblePickUpObject()
{
    drawEntity(m_getObjectMemComponents.first, m_getObjectMemComponents.second);
}

//===================================================================
void ColorDisplaySystem::setTransition(uint32_t current, uint32_t total)
{
    float currentTransparency = static_cast<float>(current) / static_cast<float>(total);
    std::get<3>(m_transitionMemComponents.second->m_vertex[0]) = currentTransparency;
    std::get<3>(m_transitionMemComponents.second->m_vertex[1]) = currentTransparency;
    std::get<3>(m_transitionMemComponents.second->m_vertex[2]) = currentTransparency;
    std::get<3>(m_transitionMemComponents.second->m_vertex[3]) = currentTransparency;
    drawEntity(m_transitionMemComponents.first, m_transitionMemComponents.second);
}

//===================================================================
void ColorDisplaySystem::display()const
{
    m_shader->display();
}

