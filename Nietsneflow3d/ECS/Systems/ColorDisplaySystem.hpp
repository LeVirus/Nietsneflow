#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>

using PairCompPosColor_t = std::pair<PositionVertexComponent*, ColorVertexComponent*>;

class ColorDisplaySystem : public ecs::System
{
public:
    ColorDisplaySystem();
    void execSystem()override;
    void setShader(Shader &shader);
    void addColorSystemEntity(uint32_t entity);
    void addFogColorEntity(uint32_t entity);
    void drawBackgroundFog();
    void loadColorEntities(uint32_t damage, uint32_t getObject, uint32_t transition, uint32_t scratchEntity, uint32_t musicVolume, uint32_t effectVolume, uint32_t turnSensitivity);
    void drawEntity(const PositionVertexComponent *posComp, const ColorVertexComponent *colorComp);
    void drawVisibleDamage();
    void drawSoundMenuBars();
    void drawInputMenuBar();
    void drawScratchWall();
    void drawVisiblePickUpObject();
    void setTransition(uint32_t current, uint32_t total);
    void setRedTransition();
    void unsetRedTransition();
    void display()const;
    void clearEntities();
    void updateMusicVolumeBar(uint32_t volume);
    void updateEffectsVolumeBar(uint32_t volume);
    void updateTurnSensitivityBar(uint32_t turnSensitivity);
private:
    void fillVertexFromEntities();
    void drawVertex();

    /**
     * @brief drawEntity Draw a single entity from the two components
     * in parameters.
     */
    void setUsedComponents();
private:
    Shader *m_shader;
    VerticesData m_verticesData;
    PairCompPosColor_t m_transitionMemComponents = {nullptr, nullptr},
    m_damageMemComponents = {nullptr, nullptr},
    m_getObjectMemComponents = {nullptr, nullptr},
    m_menuMusicVolumeComponents = {nullptr, nullptr},
    m_menuEffectsVolumeComponents = {nullptr, nullptr},
    m_menuTurnSensitivityComponents = {nullptr, nullptr},
    m_insideWallScratchMemComponents = {nullptr, nullptr},
    m_fogComponent = {nullptr, nullptr};
};
