#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>

class ColorDisplaySystem : public ecs::System
{
public:
    ColorDisplaySystem();
    void execSystem()override;
    void setShader(Shader &shader);
    void memColorSystemBackgroundEntities(uint32_t ground, uint32_t ceiling);
    inline void memDamageEntity(uint32_t damage)
    {
        m_damage = damage;
    }
    void drawEntity(const PositionVertexComponent *posComp,
                    const ColorVertexComponent *colorComp);
    void drawVisibleDamage();

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
    uint32_t m_ground, m_ceiling, m_damage;
    Shader *m_shader;
    VerticesData m_verticesData;
};
