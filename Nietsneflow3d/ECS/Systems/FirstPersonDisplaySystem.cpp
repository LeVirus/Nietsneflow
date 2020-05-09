#include "FirstPersonDisplaySystem.hpp"
#include <CollisionUtils.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <PictureData.hpp>
#include <cmath>

//===================================================================
FirstPersonDisplaySystem::FirstPersonDisplaySystem()
{
    setUsedComponents();
}

//===================================================================
void FirstPersonDisplaySystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::VISION_COMPONENT);
}

//===================================================================
void FirstPersonDisplaySystem::execSystem()
{
    System::execSystem();
    confCompVertexMemEntities();
    drawVertex();
}

//===================================================================
void FirstPersonDisplaySystem::confCompVertexMemEntities()
{
    uint32_t numEntity = mVectNumEntity.size();
    m_numVertexToDraw.resize(numEntity);
    //treat one player
    uint32_t toRemove = 0;
    for(uint32_t i = 0; i < numEntity; ++i)
    {
        VisionComponent *visionComp = stairwayToComponentManager().
                searchComponentByType<VisionComponent>(mVectNumEntity[i], Components_e::VISION_COMPONENT);
        MoveableComponent *moveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(mVectNumEntity[i], Components_e::MOVEABLE_COMPONENT);
        MapCoordComponent *mapCompA = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        assert(visionComp);
        assert(mapCompA);
        assert(moveComp);
        float leftAngleVision = moveComp->m_degreeOrientation + (visionComp->m_coneVision / 2);
        if(leftAngleVision > 360.0f)
        {
            leftAngleVision -= 360.0f;
        }
        toRemove = 0;
        m_numVertexToDraw[i] = visionComp->m_vectVisibleEntities.size();
        m_textureNumMem.resize(m_numVertexToDraw[i]);
        for(uint32_t j = 0; j < m_numVertexToDraw[i]; ++j)
        {
            GeneralCollisionComponent *genCollComp = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(visionComp->m_vectVisibleEntities[j], Components_e::GENERAL_COLLISION_COMPONENT);
            MapCoordComponent *mapCompB = stairwayToComponentManager().
                    searchComponentByType<MapCoordComponent>(visionComp->m_vectVisibleEntities[j], Components_e::MAP_COORD_COMPONENT);
            assert(mapCompB);
            assert(genCollComp);

            pairFloat_t centerPosB = getCenterPosition(mapCompB, genCollComp, visionComp->m_vectVisibleEntities[j]);
            float distance = getDistance(mapCompA->m_absoluteMapPositionPX, centerPosB);
            if(distance > visionComp->m_distanceVisibility)
            {
                ++toRemove;
                continue;
            }

            float lateralPos = leftAngleVision - getTrigoAngle(mapCompA->m_absoluteMapPositionPX, centerPosB);
            //tmp -30.0f
            if(lateralPos < -30.0f)
            {
                //Quiq fix
                lateralPos = (leftAngleVision + 360.0f) - getTrigoAngle(mapCompA->m_absoluteMapPositionPX, centerPosB);
            }
            confVertex(visionComp->m_vectVisibleEntities[j], genCollComp, visionComp, lateralPos, distance);
            fillVertexFromEntitie(visionComp->m_vectVisibleEntities[j], j);
        }
        m_numVertexToDraw[i] -= toRemove;
    }
}

//===================================================================
void FirstPersonDisplaySystem::fillVertexFromEntitie(uint32_t numEntity, uint32_t numIteration)
{
    //use 1 vertex for 1 sprite for beginning
    if(numIteration < m_vectVerticesData.size())
    {
        m_vectVerticesData[numIteration].clear();
    }
    else
    {
        m_vectVerticesData.emplace_back(VerticesData(Shader_e::TEXTURE_S));
        assert(numIteration < m_vectVerticesData.size());
    }
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(numEntity,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(posComp);
    assert(spriteComp);
    //WARNING ONLY ONE TEXTURE USED HERE
    //WARNING NO DISTANCE TREATMENT
    m_textureNumMem[numIteration] = static_cast<Texture_t>(spriteComp->m_spriteData->m_textureNum);
    m_vectVerticesData[numIteration].loadVertexTextureComponent(*posComp, *spriteComp);
}

//===================================================================
void FirstPersonDisplaySystem::setVectTextures(std::vector<Texture> &vectTexture)
{
    m_ptrVectTexture = &vectTexture;
    m_vectVerticesData.reserve(50);
}

//===================================================================
void FirstPersonDisplaySystem::confVertex(uint32_t numEntity, GeneralCollisionComponent *genCollComp,
                                          VisionComponent *visionComp, float lateralPosDegree, float distance)
{
    PositionVertexComponent *positionComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity, Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    assert(positionComp);
    assert(visionComp);
    assert(genCollComp);
    //convert to GL context
    float lateralPosGL = (lateralPosDegree / visionComp->m_coneVision * 2.0f) - 1.0f;
    float depthPos = std::abs((distance / visionComp->m_distanceVisibility) - 1.0f);
    float halfLateralSize = depthPos / spriteComp->m_glFpsSize.first / 2;
    float halfVerticalSize = depthPos / spriteComp->m_glFpsSize.second / 2;
    positionComp->m_vertex[0].first = lateralPosGL - halfLateralSize;
    positionComp->m_vertex[0].second = halfVerticalSize;
    positionComp->m_vertex[1].first = lateralPosGL + halfLateralSize;
    positionComp->m_vertex[1].second = halfVerticalSize;
    positionComp->m_vertex[2].first = lateralPosGL + halfLateralSize;
    positionComp->m_vertex[2].second = -halfVerticalSize;
    positionComp->m_vertex[3].first = lateralPosGL - halfLateralSize;
    positionComp->m_vertex[3].second = -halfVerticalSize;
}

//===================================================================
void FirstPersonDisplaySystem::drawVertex()
{
    m_shader->use();

    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        for(uint32_t h = 0; h < m_numVertexToDraw[i]; ++h)
        {
            m_ptrVectTexture->operator[](m_textureNumMem[h]).bind();
            m_vectVerticesData[h].confVertexBuffer();
            m_vectVerticesData[h].drawElement();
        }
    }
}

//===================================================================
void FirstPersonDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

//===================================================================
pairFloat_t FirstPersonDisplaySystem::getCenterPosition(const MapCoordComponent *mapComp,
                                                        GeneralCollisionComponent *genCollComp, float numEntity)
{
    assert(mapComp);
    assert(genCollComp);
    switch (genCollComp->m_shape)
    {
    case CollisionShape_e::CIRCLE_C:
    case CollisionShape_e::SEGMENT_C://TMP
        break;
    case CollisionShape_e::RECTANGLE_C:
        RectangleCollisionComponent *rectCollComp = stairwayToComponentManager().
                searchComponentByType<RectangleCollisionComponent>(numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(rectCollComp);
        return {mapComp->m_absoluteMapPositionPX.first + rectCollComp->m_size.first / 2,
                    mapComp->m_absoluteMapPositionPX.second + rectCollComp->m_size.second / 2};
    }
    return mapComp->m_absoluteMapPositionPX;
}
