#include "MainEngine.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MapPositionComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/StaticElementComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <LevelManager.hpp>
#include <cassert>

//===================================================================
MainEngine::MainEngine()
{

}

//===================================================================
void MainEngine::init()
{
    m_ecsManager.init();
    linkSystemsToGraphicEngine();
    m_graphicEngine.confSystems();
}

//===================================================================
void MainEngine::launchLoop()
{
    do
    {
        m_graphicEngine.runIteration();
    }while(!m_graphicEngine.windowShouldClose());
}

//===================================================================
void MainEngine::loadGraphicPicture(const PictureData &picData)
{
    m_graphicEngine.loadPictureData(picData);
    loadGroundAndCeilingEntities(picData.getGroundData(), picData.getCeilingData());
}


//===================================================================
void MainEngine::loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                         const GroundCeilingData &ceilingData)
{
    GroundCeilingData const *ptr = &groundData;
    for(uint32_t i = 0; i < 2; ++i)
    {
        std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
        bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
        if(ptr->m_apparence == DisplayType_e::COLOR)
        {
            bitsetComponents[Components_e::COLOR_VERTEX_COMPONENT] = true;
        }
        else if(ptr->m_apparence == DisplayType_e::TEXTURE)
        {
            bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
        }
        else
        {
            bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
            bitsetComponents[Components_e::COLOR_VERTEX_COMPONENT] = true;
        }
        uint32_t entityNum = m_ecsManager.addEntity(bitsetComponents);
        if(i)
        {
            confGroundComponents(entityNum);
        }
        else
        {
            confCeilingComponents(entityNum);
        }
        ptr = &ceilingData;
    }
}

//===================================================================
void MainEngine::loadLevelEntities(const LevelManager &levelManager)
{
    loadPlayerEntity(levelManager.getLevel());
    loadWallEntities(levelManager);
    loadStaticElementEntities(levelManager);
}

//===================================================================
void MainEngine::loadWallEntities(const LevelManager &levelManager)
{
    const std::vector<WallData> &wallData = levelManager.getLevel().getWallData();
    for(uint32_t i = 0; i < wallData.size(); ++i)
    {
        const SpriteData &memSpriteData = levelManager.getPictureData().getSpriteData()[wallData[i].m_numSprite];
        for(uint32_t j = 0; j < wallData[i].m_TileGamePosition.size(); ++j)
        {
            confBaseMapComponent(createWallEntity(),
                                 memSpriteData,
                                 wallData[i].m_TileGamePosition[j]);
        }
    }
}

//===================================================================
uint32_t MainEngine::createWallEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_POSITION_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createStaticEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_POSITION_COMPONENT] = true;
    bitsetComponents[Components_e::STATIC_ELEMENT_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
void MainEngine::confBaseMapComponent(uint32_t entityNum,
                                      const SpriteData &memSpriteData,
                                      const pairUI_t& coordLevel)
{
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(entityNum, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    spriteComp->m_spriteData = &memSpriteData;
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
            searchComponentByType<MapCoordComponent>(entityNum, Components_e::MAP_POSITION_COMPONENT);
    assert(mapComp);
    mapComp->m_coord = coordLevel;
}

//===================================================================
void MainEngine::confStaticMapComponent(uint32_t entityNum,
                                        const pairFloat_t& elementSize,
                                        bool traversable,
                                        LevelStaticElementType_e type)
{
    StaticElementComponent *staticComp = m_ecsManager.getComponentManager().
            searchComponentByType<StaticElementComponent>(entityNum, Components_e::STATIC_ELEMENT_COMPONENT);
    assert(staticComp);
    staticComp->m_inGameSpriteSize = elementSize;
    staticComp->m_traversable = traversable;
    staticComp->m_type = type;
}

//===================================================================
void MainEngine::loadPlayerEntity(const Level &level)
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_POSITION_COMPONENT] = true;
    bitsetComponents[Components_e::MOVEABLE_COMPONENT] = true;
    bitsetComponents[Components_e::COLOR_VERTEX_COMPONENT] = true;
    uint32_t entityNum = m_ecsManager.addEntity(bitsetComponents);
    confPlayerEntity(entityNum, level);
    //notify player entity number
    m_graphicEngine.getMapDisplaySystem().setPlayerEntityNum(entityNum);
}

//===================================================================
void MainEngine::confPlayerEntity(uint32_t entityNum, const Level &level)
{
    PositionVertexComponent *pos = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    MapCoordComponent *map = m_ecsManager.getComponentManager().
            searchComponentByType<MapCoordComponent>(entityNum,
                                                     Components_e::MAP_POSITION_COMPONENT);
    MoveableComponent *move = m_ecsManager.getComponentManager().
            searchComponentByType<MoveableComponent>(entityNum,
                                                     Components_e::MOVEABLE_COMPONENT);
    ColorVertexComponent *color = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent>(entityNum,
                                                     Components_e::COLOR_VERTEX_COMPONENT);
    assert(pos);
    assert(map);
    assert(move);
    map->m_coord = level.getPlayerDeparture();
    Direction_e playerDir = level.getPlayerDepartureDirection();
    switch(playerDir)
    {
    case Direction_e::NORTH:
        move->m_degreeOrientation = 0.0f;
        break;
    case Direction_e::EAST:
        move->m_degreeOrientation = 90.0f;
        break;
    case Direction_e::SOUTH:
        move->m_degreeOrientation = 180.0f;
        break;
    case Direction_e::WEST:
        move->m_degreeOrientation = 270.0f;
        break;
    }
    move->m_absoluteMapPosition = Level::getAbsolutePosition(map->m_coord);
    Level::updateOrientation(*move, *pos);//A implémenter
    //TMP
    pos->m_vertex.reserve(3);
    pos->m_vertex.emplace_back(0.75f,-0.70f);//FORWARD
    pos->m_vertex.emplace_back(0.70f,-0.80f);
    pos->m_vertex.emplace_back(0.80f,-0.80f);
    //TMP

    color->m_vertex.reserve(3);
    color->m_vertex.emplace_back(0.9f,0.00f, 0.00f);//FORWARD
    color->m_vertex.emplace_back(0.9f,0.00f, 0.00f);//FORWARD
    color->m_vertex.emplace_back(0.9f,0.00f, 0.00f);//FORWARD
}

//===================================================================
void MainEngine::loadStaticElementEntities(const LevelManager &levelManager)
{
    std::vector<StaticLevelElementData> const *staticData =
            &levelManager.getLevel().getGroundElementData();//0
    for(uint32_t h = 0; h < 3; ++h)
    {
        for(uint32_t i = 0; i < staticData->size(); ++i)
        {
            const SpriteData &memSpriteData = levelManager.getPictureData().
                    getSpriteData()[staticData->operator[](i).m_numSprite];
            for(uint32_t j = 0; j < staticData->operator[](i).m_TileGamePosition.size(); ++j)
            {
                uint32_t entityNum = createStaticEntity();
                confBaseMapComponent(entityNum,
                                     memSpriteData,
                                     staticData->operator[](i).m_TileGamePosition[j]);
                confStaticMapComponent(entityNum,
                                       staticData->operator[](i).m_inGameSpriteSize,
                                       staticData->operator[](i).m_traversable,
                                       static_cast<LevelStaticElementType_e>(h));
            }
        }
        if(!h)//h == 1
        {
            staticData = &levelManager.getLevel().getCeilingElementData();
        }
        else// h == 2
        {
            staticData = &levelManager.getLevel().getObjectElementData();
        }
    }
}

//===================================================================
void MainEngine::confGroundComponents(uint32_t entityNum)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex.resize(4);
    posComp->m_vertex[0].first  = -1.0f;
    posComp->m_vertex[0].second = 0.0f;
    posComp->m_vertex[1].first  = 1.0f;
    posComp->m_vertex[1].second = 0.0f;
    posComp->m_vertex[2].first  = 1.0f;
    posComp->m_vertex[2].second = -1.0f;
    posComp->m_vertex[3].first  = -1.0f;
    posComp->m_vertex[3].second = -1.0f;

    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent>(entityNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(colorComp);
    colorComp->m_vertex.resize(4);
    std::get<0>(colorComp->m_vertex[0]) = 0.1f;
    std::get<1>(colorComp->m_vertex[0]) = 0.2f;
    std::get<2>(colorComp->m_vertex[0]) = 0.3f;
    std::get<0>(colorComp->m_vertex[1]) = 1.0f;
    std::get<1>(colorComp->m_vertex[1]) = 0.8f;
    std::get<2>(colorComp->m_vertex[1]) = 0.1f;
    std::get<0>(colorComp->m_vertex[2]) = 0.3f;
    std::get<1>(colorComp->m_vertex[2]) = 0.9f;
    std::get<2>(colorComp->m_vertex[2]) = 0.8f;
    std::get<0>(colorComp->m_vertex[3]) = 0.6f;
    std::get<1>(colorComp->m_vertex[3]) = 0.5f;
    std::get<2>(colorComp->m_vertex[3]) = 0.1f;
}

//===================================================================
void MainEngine::confCeilingComponents(uint32_t entityNum)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex.resize(4);
    posComp->m_vertex[0].first  = -1.0f;
    posComp->m_vertex[0].second = 1.0f;
    posComp->m_vertex[1].first  = 1.0f;
    posComp->m_vertex[1].second = 1.0f;
    posComp->m_vertex[2].first  = 1.0f;
    posComp->m_vertex[2].second = 0.0f;
    posComp->m_vertex[3].first  = -1.0f;
    posComp->m_vertex[3].second = 0.0f;

    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent>(entityNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(colorComp);
    colorComp->m_vertex.resize(4);
    std::get<0>(colorComp->m_vertex[0]) = 0.5f;
    std::get<1>(colorComp->m_vertex[0]) = 0.5f;
    std::get<2>(colorComp->m_vertex[0]) = 0.5f;
    std::get<0>(colorComp->m_vertex[1]) = 0.0f;
    std::get<1>(colorComp->m_vertex[1]) = 0.5f;
    std::get<2>(colorComp->m_vertex[1]) = 0.5f;
    std::get<0>(colorComp->m_vertex[2]) = 0.5f;
    std::get<1>(colorComp->m_vertex[2]) = 0.5f;
    std::get<2>(colorComp->m_vertex[2]) = 0.0f;
    std::get<0>(colorComp->m_vertex[3]) = 0.5f;
    std::get<1>(colorComp->m_vertex[3]) = 0.0f;
    std::get<2>(colorComp->m_vertex[3]) = 0.5f;
}

//===================================================================
void MainEngine::linkSystemsToGraphicEngine()
{
    ColorDisplaySystem *color = m_ecsManager.getSystemManager().
            searchSystemByType<ColorDisplaySystem>(Systems_e::COLOR_DISPLAY_SYSTEM);
    MapDisplaySystem *map = m_ecsManager.getSystemManager().
            searchSystemByType<MapDisplaySystem>(Systems_e::MAP_DISPLAY_SYSTEM);

    m_graphicEngine.linkSystems(color, map);
}
