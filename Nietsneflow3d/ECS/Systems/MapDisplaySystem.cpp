#include "MapDisplaySystem.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapPositionComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <constants.hpp>

//===================================================================
MapDisplaySystem::MapDisplaySystem():m_verticesData(Shader_e::TEXTURE_S)
{
    setUsedComponents();
}

//===================================================================
void MapDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

//===================================================================
void MapDisplaySystem::execSystem()
{
    System::execSystem();
//    confEntity();
    fillVertexFromEntities();
    drawVertex();
}

//===================================================================
void MapDisplaySystem::fillVertexFromEntities()
{
    m_verticesData.clear();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(mVectNumEntity[i],
                                                               Components_e::POSITION_VERTEX_COMPONENT);
        //TEST
        posComp->m_vertex.resize(4);
        posComp->m_vertex[0].first  = -1.0f;
        posComp->m_vertex[0].second = 0.0f;
        posComp->m_vertex[1].first  = 1.0f;
        posComp->m_vertex[1].second = 0.0f;
        posComp->m_vertex[2].first  = 1.0f;
        posComp->m_vertex[2].second = -1.0f;
        posComp->m_vertex[3].first  = -1.0f;
        posComp->m_vertex[3].second = -1.0f;
        //TEST
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(mVectNumEntity[i],
                                                            Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(posComp);
        assert(spriteComp);
        m_verticesData.loadVertexTextureComponent(*posComp, *spriteComp);
    }
}

//===================================================================
void MapDisplaySystem::drawVertex()
{
    m_verticesData.confVertexBuffer();
    m_shader->use();
    m_verticesData.drawElement();
}

//===================================================================
void MapDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::POSITION_VERTEX_COMPONENT);
    bAddComponentToSystem(Components_e::SPRITE_TEXTURE_COMPONENT);
    bAddComponentToSystem(Components_e::MAP_POSITION_COMPONENT);
}
