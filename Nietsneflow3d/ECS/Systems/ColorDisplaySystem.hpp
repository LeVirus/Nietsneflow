#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>

using PairCompPosColor_t = std::pair<PositionVertexComponent *, ColorVertexComponent*>;

class ColorDisplaySystem : public ecs::System
{
public:
    ColorDisplaySystem();
    void execSystem()override;
    void setShader(Shader &shader);
    void memColorSystemBackgroundEntities(uint32_t ground, uint32_t ceiling);
    void loadDamageEntity(uint32_t damage);
    void loadTransitionEntity(uint32_t transition);
    void drawEntity(const PositionVertexComponent *posComp, const ColorVertexComponent *colorComp);
    void drawVisibleDamage();
    void setTransition(uint32_t current, uint32_t total);
    void display()const;
private:
    void fillVertexFromEntities();
    void drawVertex();

    /**
     * @brief drawEntity Draw a single entity from the two components
     * in parameters.
     */
    void setUsedComponents();
private:
    uint32_t m_ground, m_ceiling, m_damage, m_transition;
    Shader *m_shader;
    VerticesData m_verticesData;
    PairCompPosColor_t m_transitionMemComponents = {nullptr, nullptr},
    m_damageMemComponents = {nullptr, nullptr};
};
