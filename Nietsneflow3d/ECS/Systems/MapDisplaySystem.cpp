#include "MapDisplaySystem.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapPositionComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <constants.hpp>

//===================================================================
MapDisplaySystem::MapDisplaySystem():m_verticesData(Shader_e::TEXTURE_S)
{
    setUsedComponents();
}


//===================================================================
void MapDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::POSITION_VERTEX_COMPONENT);
    bAddComponentToSystem(Components_e::SPRITE_TEXTURE_COMPONENT);
    bAddComponentToSystem(Components_e::MAP_POSITION_COMPONENT);
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
    confEntity();
    fillVertexFromEntities();
    drawVertex();
    drawPlayerOnMap();
}

//===================================================================
void MapDisplaySystem::confEntity()
{
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
//        PositionVertexComponent *posComp = stairwayToComponentManager().
//                searchComponentByType<PositionVertexComponent>(mVectNumEntity[i],
//                                                               Components_e::POSITION_VERTEX_COMPONENT);
//        //TEST
//        posComp->m_vertex.reserve(4);
//        posComp->m_vertex[0].first  = -1.0f;
//        posComp->m_vertex[0].second = 0.0f;
//        posComp->m_vertex[1].first  = 1.0f;
//        posComp->m_vertex[1].second = 0.0f;
//        posComp->m_vertex[2].first  = 1.0f;
//        posComp->m_vertex[2].second = -1.0f;
//        posComp->m_vertex[3].first  = -1.0f;
//        posComp->m_vertex[3].second = -1.0f;
    }
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
//        posComp->m_vertex.emplace_back(-1.0f, 0.0f);
//        posComp->m_vertex.emplace_back(1.0f, 0.0f);
//        posComp->m_vertex.emplace_back(1.0f, -1.0f);
//        posComp->m_vertex.emplace_back(-1.0f, -1.0f);
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
    drawPlayerOnMap();
}

//===================================================================
void MapDisplaySystem::drawPlayerOnMap()
{
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(m_playerNum,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    ColorVertexComponent *colorComp = stairwayToComponentManager().
            searchComponentByType<ColorVertexComponent>(m_playerNum,
                                                           Components_e::COLOR_VERTEX_COMPONENT);
    assert(posComp);
    assert(colorComp);
    mptrSystemManager->searchSystemByType<ColorDisplaySystem>(
                Systems_e::COLOR_DISPLAY_SYSTEM)->drawEntity(posComp, colorComp);
}
