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
void ColorDisplaySystem::memColorSystemBackgroundEntities(uint32_t ground, uint32_t ceiling)
{
    m_ground = ground;
    m_ceiling = ceiling;
    mVectNumEntity.reserve(2);
    mVectNumEntity.emplace_back(m_ground);
    mVectNumEntity.emplace_back(m_ceiling);
}

//===================================================================
void ColorDisplaySystem::drawEntity(const PositionVertexComponent *posComp,
                                    const ColorVertexComponent *colorComp)
{
    m_verticesData.clear();
    m_verticesData.loadVertexColorComponent(posComp, colorComp);
    drawVertex();
}

//===================================================================
void ColorDisplaySystem::drawVisibleDamage()
{
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(m_damage,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    ColorVertexComponent *colorComp = stairwayToComponentManager().
            searchComponentByType<ColorVertexComponent>(m_damage,
                                                        Components_e::COLOR_VERTEX_COMPONENT);
    assert(posComp);
    assert(colorComp);
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::display() const
{
    m_shader->display();
}

