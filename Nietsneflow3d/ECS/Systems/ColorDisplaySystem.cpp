#include "ColorDisplaySystem.hpp"
#include <constants.hpp>
#include <includesLib/BaseECS/engine.hpp>
#include <ECS/Components/NumberVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <cassert>
#include <OpenGLUtils/glheaders.hpp>

//===================================================================
ColorDisplaySystem::ColorDisplaySystem():m_verticesData(Shader_e::CEILING_FLOOR)
{
}

//===================================================================
void ColorDisplaySystem::fillVertexFromEntities()
{
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        NumberVertexComponent *numVertexComp = stairwayToComponentManager().
                searchComponentByType<NumberVertexComponent>(mVectNumEntity[i],
                                                             Components_e::NUMBER_VERTEX_COMPONENT);
        assert(numVertexComp);
        size_t numVertex = numVertexComp->m_numberVertex;
        if(numVertex == 3)
        {

            PositionVertexComponent<3> *posComp = stairwayToComponentManager().
                    searchComponentByType<PositionVertexComponent<3>>(mVectNumEntity[i],
                                                                              Components_e::POSITION_VERTEX_COMPONENT);
            ColorVertexComponent<3> *colorComp = stairwayToComponentManager().
                    searchComponentByType<ColorVertexComponent<3>>(mVectNumEntity[i],
                                                                           Components_e::COLOR_VERTEX_COMPONENT);
            assert(posComp);
            assert(colorComp);
            m_verticesData.loadVertexComponent(posComp, colorComp);
        }
        else if(numVertex == 4)
        {

            PositionVertexComponent<4> *posComp = stairwayToComponentManager().
                    searchComponentByType<PositionVertexComponent<4>>(mVectNumEntity[i],
                                                                              Components_e::POSITION_VERTEX_COMPONENT);
            ColorVertexComponent<4> *colorComp = stairwayToComponentManager().
                    searchComponentByType<ColorVertexComponent<4>>(mVectNumEntity[i],
                                                                           Components_e::COLOR_VERTEX_COMPONENT);
            assert(posComp);
            assert(colorComp);
            m_verticesData.loadVertexComponent(posComp, colorComp);
        }
    }
}

//===================================================================
void ColorDisplaySystem::drawVertex()
{
    uint32_t indices[] = {
        0, 1, 3,   // premier triangle
        1, 2, 3,   // second triangle
        4, 5, 7,   // premier triangle
        5, 6, 7    // second triangle
    };
    uint32_t EBO;
    glGenBuffers(1, &EBO);

    uint32_t VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_verticesData.getvectVertex().size(),
                 &m_verticesData.getvectVertex()[0], GL_STATIC_DRAW);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
    m_shader->use();

    // draw our first triangle
    glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
//    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//    glDrawArrays(GL_TRIANGLES, 0, 4);
//    glDrawArrays(GL_TRIANGLES, 4, 4);

    glBindVertexArray(0); // no need to unbind it every time
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
    fillVertexFromEntities();
    drawVertex();
}

//===================================================================
void ColorDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

//===================================================================
void ColorDisplaySystem::display() const
{
    std::cerr << "vertex\n";
    for(uint32_t i = 0; i < m_vectVertex.size(); ++i)
    {
        std::cerr << m_vectVertex[i] << "  " <<
                     m_vectVertex[++i] << "  " <<
                     m_vectVertex[++i] << "  " <<
                     m_vectVertex[++i] << "  " <<
                     m_vectVertex[++i] << "\n";
    }
    m_shader->display();
}

