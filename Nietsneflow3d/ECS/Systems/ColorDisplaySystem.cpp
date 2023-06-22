#include "ColorDisplaySystem.hpp"
#include <constants.hpp>
#include <includesLib/BaseECS/engine.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <cassert>

//===================================================================
ColorDisplaySystem::ColorDisplaySystem(NewComponentManager &newComponentManager) : m_newComponentManager(newComponentManager),
    m_componentsContainer(m_newComponentManager.getComponentsContainer()),
    m_verticesData(Shader_e::COLOR_S)
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
        PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(mVectNumEntity[i]);
        ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(mVectNumEntity[i]);
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
void ColorDisplaySystem::addFogColorEntity(uint32_t entity)
{
    m_fogNum = entity;
}

//===================================================================
void ColorDisplaySystem::loadColorEntities(uint32_t damage, uint32_t getObject, uint32_t transition, uint32_t scratchEntity,
                                           uint32_t musicVolume, uint32_t effectVolume, uint32_t turnSensitivity)
{
    m_damageNum = damage;
    m_getObjectNum = getObject;
    m_transitionNum = transition;
    m_insideWallScratchMemNum = scratchEntity;
    m_menuMusicVolumeNum = musicVolume;
    m_menuEffectsVolumeNum = effectVolume;
    m_menuTurnSensitivityNum = turnSensitivity;
}

//===================================================================
void ColorDisplaySystem::drawEntity(const PositionVertexComponent *posComp, const ColorVertexComponent *colorComp)
{
    m_verticesData.clear();
    m_verticesData.loadVertexColorComponent(posComp, colorComp);
    drawVertex();
}

//===================================================================
void ColorDisplaySystem::drawBackgroundFog()
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_fogNum);
    ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(*m_fogNum);
    assert(posComp);
    assert(colorComp);
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::drawVisibleDamage()
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_damageNum);
    ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(*m_damageNum);
    assert(posComp);
    assert(colorComp);
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::drawSoundMenuBars()
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_menuMusicVolumeNum);
    ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(*m_menuMusicVolumeNum);
    assert(posComp);
    assert(colorComp);
    drawEntity(posComp, colorComp);
    posComp = m_newComponentManager.getPosVertexComponent(*m_menuEffectsVolumeNum);
    colorComp = m_newComponentManager.getColorVertexComponent(*m_menuEffectsVolumeNum);
    assert(posComp);
    assert(colorComp);
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::drawInputMenuBar()
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_menuTurnSensitivityNum);
    ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(*m_menuTurnSensitivityNum);
    assert(posComp);
    assert(colorComp);
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::drawScratchWall()
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_insideWallScratchMemNum);
    ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(*m_insideWallScratchMemNum);
    assert(posComp);
    assert(colorComp);
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::drawVisiblePickUpObject()
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_getObjectNum);
    ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(*m_getObjectNum);
    assert(posComp);
    assert(colorComp);
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::setTransition(uint32_t current, uint32_t total)
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_transitionNum);
    ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(*m_transitionNum);
    assert(posComp);
    assert(colorComp);
    drawEntity(posComp, colorComp);
    float currentTransparency = static_cast<float>(current) / static_cast<float>(total);
    std::get<3>(colorComp->m_vertex[0]) = currentTransparency;
    std::get<3>(colorComp->m_vertex[1]) = currentTransparency;
    std::get<3>(colorComp->m_vertex[2]) = currentTransparency;
    std::get<3>(colorComp->m_vertex[3]) = currentTransparency;
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::setRedTransition()
{
    ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(*m_transitionNum);
    assert(colorComp);
    std::get<0>(colorComp->m_vertex[0]) = 0.8f;
    std::get<0>(colorComp->m_vertex[1]) = 0.8f;
    std::get<0>(colorComp->m_vertex[2]) = 0.8f;
    std::get<0>(colorComp->m_vertex[3]) = 0.8f;
}

//===================================================================
void ColorDisplaySystem::unsetRedTransition()
{
    ColorVertexComponent *colorComp = m_newComponentManager.getColorVertexComponent(*m_transitionNum);
    assert(colorComp);
    std::get<0>(colorComp->m_vertex[0]) = 0.0f;
    std::get<0>(colorComp->m_vertex[1]) = 0.0f;
    std::get<0>(colorComp->m_vertex[2]) = 0.0f;
    std::get<0>(colorComp->m_vertex[3]) = 0.0f;
}

//===================================================================
void ColorDisplaySystem::display()const
{
    m_shader->display();
}

//===================================================================
void ColorDisplaySystem::clearEntities()
{
    mVectNumEntity.clear();
}

//===================================================================
void ColorDisplaySystem::updateMusicVolumeBar(uint32_t volume)
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_menuMusicVolumeNum);
    assert(posComp);
    float newVal = LEFT_POS_STD_MENU_BAR + 0.01f + (volume * MAX_BAR_MENU_SIZE) / 100.0f;
    posComp->m_vertex[1].first = newVal;
    posComp->m_vertex[2].first = newVal;
}

//===================================================================
void ColorDisplaySystem::updateEffectsVolumeBar(uint32_t volume)
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_menuEffectsVolumeNum);
    assert(posComp);
    float newVal = LEFT_POS_STD_MENU_BAR + 0.01f + (volume * MAX_BAR_MENU_SIZE) / 100.0f;
    posComp->m_vertex[1].first = newVal;
    posComp->m_vertex[2].first = newVal;
}

//===================================================================
void ColorDisplaySystem::updateTurnSensitivityBar(uint32_t turnSensitivity)
{
    PositionVertexComponent *posComp = m_newComponentManager.getPosVertexComponent(*m_menuTurnSensitivityNum);
    assert(posComp);
    float newVal = LEFT_POS_STD_MENU_BAR + 0.01f + ((turnSensitivity - MIN_TURN_SENSITIVITY) * MAX_BAR_MENU_SIZE) / DIFF_TOTAL_SENSITIVITY;
    posComp->m_vertex[1].first = newVal;
    posComp->m_vertex[2].first = newVal;
}
