#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <ECS/NewComponentManager.hpp>

using OptUint_t = std::optional<uint32_t>;

class ColorDisplaySystem : public ecs::System
{
public:
    ColorDisplaySystem(NewComponentManager &newComponentManager);
    void execSystem()override;
    void setShader(Shader &shader);
    void addColorSystemEntity(uint32_t entity);
    void addFogColorEntity(uint32_t entity);
    void drawBackgroundFog();
    void loadColorEntities(uint32_t damage, uint32_t getObject, uint32_t transition, uint32_t scratchEntity, uint32_t musicVolume, uint32_t effectVolume, uint32_t turnSensitivity);
    void drawEntity(const PositionVertexComponent &posComp, const ColorVertexComponent &colorComp);
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
    NewComponentManager &m_newComponentManager;
    ComponentsGroup &m_componentsContainer;
    Shader *m_shader;
    VerticesData m_verticesData;
    OptUint_t m_transitionNum = std::nullopt,
    m_damageNum = std::nullopt,
    m_getObjectNum = std::nullopt,
    m_menuMusicVolumeNum = std::nullopt,
    m_menuEffectsVolumeNum = std::nullopt,
    m_menuTurnSensitivityNum = std::nullopt,
    m_insideWallScratchMemNum = std::nullopt,
    m_fogNum = std::nullopt;
};
