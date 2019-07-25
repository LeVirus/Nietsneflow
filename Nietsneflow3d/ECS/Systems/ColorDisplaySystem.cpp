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
    m_verticesData.clear();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        NumberVertexComponent *numVertexComp = stairwayToComponentManager().
                searchComponentByType<NumberVertexComponent>(mVectNumEntity[i],
                                                             Components_e::NUMBER_VERTEX_COMPONENT);
        assert(numVertexComp);
        BaseShapeType_e shapeType = numVertexComp->m_shapeType;
        if(shapeType == BaseShapeType_e::TRIANGLE)
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
        else if(shapeType == BaseShapeType_e::RECTANGLE)
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
    m_verticesData.bindGLBuffers();
    m_verticesData.attribGLVertexPointer();
    m_shader->use();
    m_verticesData.drawElement();
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
    m_shader->display();
}

