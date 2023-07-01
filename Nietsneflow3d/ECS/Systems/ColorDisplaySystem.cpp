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
    OptUint_t compNum;
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        compNum = m_newComponentManager.getComponentEmplacement(mVectNumEntity[i], Components_e::POSITION_VERTEX_COMPONENT);
        assert(compNum);
        PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
        compNum = m_newComponentManager.getComponentEmplacement(mVectNumEntity[i], Components_e::COLOR_VERTEX_COMPONENT);
        assert(compNum);
        ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
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
void ColorDisplaySystem::drawEntity(const PositionVertexComponent &posComp, const ColorVertexComponent &colorComp)
{
    m_verticesData.clear();
    m_verticesData.loadVertexColorComponent(posComp, colorComp);
    drawVertex();
}

//===================================================================
void ColorDisplaySystem::drawBackgroundFog()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_fogNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(*m_fogNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::drawVisibleDamage()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_damageNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(*m_damageNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::drawSoundMenuBars()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_menuMusicVolumeNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(*m_menuMusicVolumeNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    drawEntity(posComp, colorComp);
    compNum = m_newComponentManager.getComponentEmplacement(*m_menuEffectsVolumeNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posCompA = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(*m_menuEffectsVolumeNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorCompA = m_componentsContainer.m_vectColorVertexComp[*compNum];
    drawEntity(posCompA, colorCompA);
}

//===================================================================
void ColorDisplaySystem::drawInputMenuBar()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_menuTurnSensitivityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(*m_menuTurnSensitivityNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::drawScratchWall()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_insideWallScratchMemNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(*m_insideWallScratchMemNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::drawVisiblePickUpObject()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_getObjectNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(*m_getObjectNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::setTransition(uint32_t current, uint32_t total)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_transitionNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    compNum = m_newComponentManager.getComponentEmplacement(*m_transitionNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    drawEntity(posComp, colorComp);
    float currentTransparency = static_cast<float>(current) / static_cast<float>(total);
    std::get<3>(colorComp.m_vertex[0]) = currentTransparency;
    std::get<3>(colorComp.m_vertex[1]) = currentTransparency;
    std::get<3>(colorComp.m_vertex[2]) = currentTransparency;
    std::get<3>(colorComp.m_vertex[3]) = currentTransparency;
    drawEntity(posComp, colorComp);
}

//===================================================================
void ColorDisplaySystem::setRedTransition()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_transitionNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    std::get<0>(colorComp.m_vertex[0]) = 0.8f;
    std::get<0>(colorComp.m_vertex[1]) = 0.8f;
    std::get<0>(colorComp.m_vertex[2]) = 0.8f;
    std::get<0>(colorComp.m_vertex[3]) = 0.8f;
}

//===================================================================
void ColorDisplaySystem::unsetRedTransition()
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_transitionNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(compNum);
    ColorVertexComponent &colorComp = m_componentsContainer.m_vectColorVertexComp[*compNum];
    std::get<0>(colorComp.m_vertex[0]) = 0.0f;
    std::get<0>(colorComp.m_vertex[1]) = 0.0f;
    std::get<0>(colorComp.m_vertex[2]) = 0.0f;
    std::get<0>(colorComp.m_vertex[3]) = 0.0f;
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
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_menuMusicVolumeNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    float newVal = LEFT_POS_STD_MENU_BAR + 0.01f + (volume * MAX_BAR_MENU_SIZE) / 100.0f;
    posComp.m_vertex[1].first = newVal;
    posComp.m_vertex[2].first = newVal;
}

//===================================================================
void ColorDisplaySystem::updateEffectsVolumeBar(uint32_t volume)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_menuEffectsVolumeNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    float newVal = LEFT_POS_STD_MENU_BAR + 0.01f + (volume * MAX_BAR_MENU_SIZE) / 100.0f;
    posComp.m_vertex[1].first = newVal;
    posComp.m_vertex[2].first = newVal;
}

//===================================================================
void ColorDisplaySystem::updateTurnSensitivityBar(uint32_t turnSensitivity)
{
    OptUint_t compNum = m_newComponentManager.getComponentEmplacement(*m_menuTurnSensitivityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(compNum);
    PositionVertexComponent &posComp = m_componentsContainer.m_vectPositionVertexComp[*compNum];
    float newVal = LEFT_POS_STD_MENU_BAR + 0.01f + ((turnSensitivity - MIN_TURN_SENSITIVITY) * MAX_BAR_MENU_SIZE) / DIFF_TOTAL_SENSITIVITY;
    posComp.m_vertex[1].first = newVal;
    posComp.m_vertex[2].first = newVal;
}
