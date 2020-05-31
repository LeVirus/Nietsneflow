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
        m_entitiesNumMem.clear();
        for(uint32_t j = 0; j < m_numVertexToDraw[i]; ++j)
        {
            GeneralCollisionComponent *genCollComp = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(visionComp->m_vectVisibleEntities[j],
                                                                     Components_e::GENERAL_COLLISION_COMPONENT);
            MapCoordComponent *mapCompB = stairwayToComponentManager().
                    searchComponentByType<MapCoordComponent>(visionComp->m_vectVisibleEntities[j],
                                                             Components_e::MAP_COORD_COMPONENT);
            assert(mapCompB);
            assert(genCollComp);
            treatDisplayEntity(genCollComp, mapCompA, mapCompB, visionComp, toRemove, leftAngleVision, j);
        }
        m_numVertexToDraw[i] -= toRemove;
    }
}

//===================================================================
void FirstPersonDisplaySystem::treatDisplayEntity(GeneralCollisionComponent *genCollComp, MapCoordComponent *mapCompA,
                                                  MapCoordComponent *mapCompB, VisionComponent *visionComp,
                                                  uint32_t &toRemove, float leftAngleVision, uint32_t numIteration)
{
    if(genCollComp->m_tag == CollisionTag_e::WALL_CT)
    {

        float distance[4];
        pairFloat_t absolPos[4];
        float lateralPos[3];
        fillWallEntitiesData(visionComp->m_vectVisibleEntities[numIteration], absolPos, distance,
                             mapCompA, mapCompB);
        //calculate all 3 display position
        for(uint32_t i = 0; i < 3; ++i)
        {
            lateralPos[i] = leftAngleVision - getTrigoAngle(mapCompA->m_absoluteMapPositionPX, absolPos[i]);
            //tmp -30.0f
            if(lateralPos[i] < -30.0f)
            {
                //Quick fix
                lateralPos[i] = (leftAngleVision + 360.0f) - getTrigoAngle(mapCompA->m_absoluteMapPositionPX, absolPos[i]);
            }
        }
        confWallEntityVertex(visionComp->m_vectVisibleEntities[numIteration], visionComp, lateralPos, distance);
        fillVertexFromEntitie(visionComp->m_vectVisibleEntities[numIteration], numIteration, distance[2]);
    }
    else
    {
        pairFloat_t centerPosB = getCenterPosition(mapCompB, genCollComp, visionComp->m_vectVisibleEntities[numIteration]);
        float distance = getDistance(mapCompA->m_absoluteMapPositionPX, centerPosB);
        if(distance > visionComp->m_distanceVisibility)
        {
            ++toRemove;
            return;
        }

        float lateralPos = leftAngleVision - getTrigoAngle(mapCompA->m_absoluteMapPositionPX, centerPosB);
        //tmp -30.0f
        if(lateralPos < -30.0f)
        {
            //Quick fix
            lateralPos = (leftAngleVision + 360.0f) - getTrigoAngle(mapCompA->m_absoluteMapPositionPX, centerPosB);
        }
        confNormalEntityVertex(visionComp->m_vectVisibleEntities[numIteration], visionComp, lateralPos, distance);
        fillVertexFromEntitie(visionComp->m_vectVisibleEntities[numIteration], numIteration, distance);
    }
}

//===================================================================
void FirstPersonDisplaySystem::confWallEntityVertex(uint32_t numEntity, VisionComponent *visionComp,
                                                    float lateralPosDegree[], float distance[])
{
    PositionVertexComponent *positionComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity, Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    assert(positionComp);
    assert(visionComp);

    positionComp->m_vertex.resize(6);
    uint32_t first = 0, last = 2;
    if(lateralPosDegree[0] < lateralPosDegree[2])
    {
        first = 2;
        last = 0;
        std::cerr << "DDDDDDDDDDDD\n";
    }
    //convert to GL context
    float lateralPosGL = (lateralPosDegree[first] / visionComp->m_coneVision * 2.0f) - 1.0f;
    float depthPos = std::abs((distance[first] / visionComp->m_distanceVisibility) - 1.0f);
    float halfVerticalSize = depthPos / spriteComp->m_glFpsSize.second / 2;
    //convert to GL context
    float lateralPosGLMid = (lateralPosDegree[1] / visionComp->m_coneVision * 2.0f) - 1.0f;
    float depthPosMid = std::abs((distance[1] / visionComp->m_distanceVisibility) - 1.0f);
    float halfVerticalSizeMid = depthPosMid / spriteComp->m_glFpsSize.second / 2;

    float lateralPosMaxGL = (lateralPosDegree[last] / visionComp->m_coneVision * 2.0f) - 1.0f;
    float depthPosMax = std::abs((distance[last] / visionComp->m_distanceVisibility) - 1.0f);
    float halfVerticalSizeMax = depthPosMax / spriteComp->m_glFpsSize.second / 2;

    std::cerr << "lateralPosGL " << lateralPosGL
                  << " lateralPosGLMid " << lateralPosGLMid
                      << " lateralPosMaxGL " << lateralPosMaxGL << "\n\n";
    std::cerr << "distance1 " << distance[first]
                  << " distanceMid " << distance[1]
                      << " distance3 " << distance[last]
                         << " distance4 " << distance[3] << "\n\n";
//    visionComp-> <<

    positionComp->m_vertex[0].first = lateralPosGL;
    positionComp->m_vertex[0].second = halfVerticalSize;
    positionComp->m_vertex[1].first = lateralPosGLMid;
    positionComp->m_vertex[1].second = halfVerticalSizeMid;
    positionComp->m_vertex[2].first = lateralPosGLMid;
    positionComp->m_vertex[2].second = -halfVerticalSizeMid;
    positionComp->m_vertex[3].first = lateralPosGL;
    positionComp->m_vertex[3].second = -halfVerticalSize;

    positionComp->m_vertex[4].first = lateralPosMaxGL;
    positionComp->m_vertex[4].second = halfVerticalSizeMax;
    positionComp->m_vertex[5].first = lateralPosMaxGL;
    positionComp->m_vertex[5].second = -halfVerticalSizeMax;
}

//===================================================================
void FirstPersonDisplaySystem::fillWallEntitiesData(uint32_t numEntity, pairFloat_t absolPos[], float distance[],
                                                    MapCoordComponent *mapCompA, MapCoordComponent *mapCompB)
{
    RectangleCollisionComponent *rectComp = stairwayToComponentManager().
            searchComponentByType<RectangleCollisionComponent>(numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
    assert(rectComp);
    //get all points WALL rect position
    //up left
    absolPos[0] = mapCompB->m_absoluteMapPositionPX;
    absolPos[1] = {mapCompB->m_absoluteMapPositionPX.first + rectComp->m_size.first,
                   mapCompB->m_absoluteMapPositionPX.second};
    absolPos[2] = {mapCompB->m_absoluteMapPositionPX.first + rectComp->m_size.first,
                   mapCompB->m_absoluteMapPositionPX.second + rectComp->m_size.second};
    absolPos[3] = {mapCompB->m_absoluteMapPositionPX.first,
                   mapCompB->m_absoluteMapPositionPX.second + rectComp->m_size.second};
    //up left
    distance[0] = getDistance(mapCompA->m_absoluteMapPositionPX, absolPos[0]);
    //up right
    distance[1] = getDistance(mapCompA->m_absoluteMapPositionPX, absolPos[1]);
    //down right
    distance[2] = getDistance(mapCompA->m_absoluteMapPositionPX, absolPos[2]);
    //down left
    distance[3] = getDistance(mapCompA->m_absoluteMapPositionPX, absolPos[3]);
    uint32_t minVal = getMinOrMaxValueFromEntries(distance, true);
    //second display
    if(minVal != 1)
    {
        std::swap(distance[minVal], distance[1]);
        std::swap(absolPos[minVal], absolPos[1]);
    }
    uint32_t maxVal = getMinOrMaxValueFromEntries(distance, false);
    //do not display
    if(maxVal != 3)
    {
        std::swap(distance[3], distance[maxVal]);
        std::swap(absolPos[3], absolPos[maxVal]);
    }
}

//===================================================================
void FirstPersonDisplaySystem::fillVertexFromEntitie(uint32_t numEntity, uint32_t numIteration, float distance)
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
    m_entitiesNumMem.insert(EntityData(distance, static_cast<Texture_t>(spriteComp->m_spriteData->m_textureNum), numIteration));

    m_vectVerticesData[numIteration].loadVertexTextureComponent(*posComp, *spriteComp);
}

//===================================================================
void FirstPersonDisplaySystem::setVectTextures(std::vector<Texture> &vectTexture)
{
    m_ptrVectTexture = &vectTexture;
    m_vectVerticesData.reserve(50);
}

//===================================================================
void FirstPersonDisplaySystem::confNormalEntityVertex(uint32_t numEntity, VisionComponent *visionComp,
                                                      float lateralPosDegree, float distance)
{
    PositionVertexComponent *positionComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(numEntity, Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    assert(positionComp);
    assert(visionComp);
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

    //DONT WORK for multiple player
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        std::set<EntityData>::const_iterator it = m_entitiesNumMem.begin();
        uint32_t numEntity;
        for(;it != m_entitiesNumMem.end(); ++it)
        {
            numEntity = it->m_entityNum;
            assert(numEntity < m_vectVerticesData.size());
            m_ptrVectTexture->operator[](it->m_textureNum).bind();
            m_vectVerticesData[numEntity].confVertexBuffer();
            m_vectVerticesData[numEntity].drawElement();
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

uint32_t getMinOrMaxValueFromEntries(const float distance[], bool min)
{
    uint32_t val = 0;
    if(min)
    {
        val = (distance[0] < distance[1]) ? 0 : 1;
        val = (distance[val] < distance[2]) ? val : 2;
        val = (distance[val] < distance[3]) ? val : 3;
    }
    else
    {
        val = (distance[0] > distance[1]) ? 0 : 1;
        val = (distance[val] > distance[2]) ? val : 2;
        val = (distance[val] > distance[3]) ? val : 3;
    }
    return val;
}
