#pragma once

#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>

class ColorDisplaySystem : public ecs::System
{
private:
    Shader *m_shader;
    VerticesData m_verticesData;
private:
    void fillVertexFromEntities();
    void drawVertex();

    /**
     * @brief drawEntity Draw a single entity from the two components
     * in parameters.
     */
    void setUsedComponents();
public:
    ColorDisplaySystem();
    void execSystem()override;
    void setShader(Shader &shader);
    void drawEntity(const PositionVertexComponent *posComp,
                    const ColorVertexComponent *colorComp);
    void display()const;
};
