#include "MainEngine.hpp"
#include "constants.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/StaticElementComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <ECS/Systems/CollisionSystem.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <ECS/Systems/VisionSystem.hpp>
#include <ECS/Systems/DoorSystem.hpp>
#include <LevelManager.hpp>
#include <cassert>

//===================================================================
void MainEngine::init()
{
    m_ecsManager.init();
    linkSystemsToGraphicEngine();
    linkSystemsToPhysicalEngine();
    m_graphicEngine.confSystems();
}

//===================================================================
void MainEngine::mainLoop()
{
    m_graphicEngine.getMapDisplaySystem().confLevelData();
    do
    {
        m_graphicEngine.runIteration();
        m_physicalEngine.runIteration();
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
    loadDoorEntities(levelManager);
    loadStaticElementEntities(levelManager);
    loadEnemiesEntities(levelManager);
}

//===================================================================
void MainEngine::loadWallEntities(const LevelManager &levelManager)
{
    const std::vector<WallData> &wallData = levelManager.getLevel().getWallData();
    MemSpriteDataComponent *memSpriteComp;
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    for(uint32_t i = 0; i < wallData.size(); ++i)
    {
        const SpriteData &memSpriteData = levelManager.getPictureData().getSpriteData()[wallData[i].m_sprites[0]];
        for(uint32_t j = 0; j < wallData[i].m_TileGamePosition.size(); ++j)
        {
            uint32_t numEntity = createWallEntity();
            confBaseComponent(numEntity, memSpriteData, wallData[i].m_TileGamePosition[j],
                              CollisionShape_e::RECTANGLE_C);
            memSpriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MemSpriteDataComponent>(numEntity,
                                                                  Components_e::MEM_SPRITE_DATA_COMPONENT);
            if(!memSpriteComp)
            {
                continue;
            }
            uint32_t vectSize = wallData[i].m_sprites.size();
            memSpriteComp->m_vectSpriteData.reserve(static_cast<uint32_t>(WallSpriteType_e::TOTAL_SPRITE));
            for(uint32_t j = 0; j < vectSize; ++j)
            {
                memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[wallData[i].m_sprites[j]]);
            }
        }
    }
}

//===================================================================
void MainEngine::loadDoorEntities(const LevelManager &levelManager)
{
    const std::vector<DoorData> &doorData = levelManager.getLevel().getDoorData();
    MemSpriteDataComponent *memSpriteComp;
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    for(uint32_t i = 0; i < doorData.size(); ++i)
    {
        const SpriteData &memSpriteData = levelManager.getPictureData().getSpriteData()[doorData[i].m_numSprite];
        for(uint32_t j = 0; j < doorData[i].m_TileGamePosition.size(); ++j)
        {
            uint32_t numEntity = createDoorEntity();
            confBaseComponent(numEntity, memSpriteData, doorData[i].m_TileGamePosition[j],
                              CollisionShape_e::RECTANGLE_C);
            GeneralCollisionComponent *tagComp = m_ecsManager.getComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(numEntity, Components_e::GENERAL_COLLISION_COMPONENT);
            assert(tagComp);
            tagComp->m_tag = CollisionTag_e::DOOR_CT;
            MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MapCoordComponent>(numEntity, Components_e::MAP_COORD_COMPONENT);
            assert(mapComp);
            RectangleCollisionComponent *rectComp = m_ecsManager.getComponentManager().
                    searchComponentByType<RectangleCollisionComponent>(numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
            assert(rectComp);
            DoorComponent *doorComp = m_ecsManager.getComponentManager().
                    searchComponentByType<DoorComponent>(numEntity, Components_e::DOOR_COMPONENT);
            assert(doorComp);
            if(doorData[i].m_vertical)
            {
                mapComp->m_absoluteMapPositionPX.first += DOOR_CASE_POS_PX;
                rectComp->m_size = {WIDTH_DOOR_SIZE_PX, LEVEL_TILE_SIZE_PX};
            }
            else
            {
                mapComp->m_absoluteMapPositionPX.second += DOOR_CASE_POS_PX;
                rectComp->m_size = {LEVEL_TILE_SIZE_PX, WIDTH_DOOR_SIZE_PX};
            }
            doorComp->m_vertical = doorData[i].m_vertical;
            memSpriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MemSpriteDataComponent>(numEntity,
                                                                  Components_e::MEM_SPRITE_DATA_COMPONENT);
            if(!memSpriteComp)
            {
                continue;
            }
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[doorData[i].m_numSprite]);
        }
    }
}

//===================================================================
void MainEngine::loadEnemiesEntities(const LevelManager &levelManager)
{
    const std::vector<EnemyData> &enemiesData = levelManager.getLevel().getEnemiesData();

    for(uint32_t i = 0; i < enemiesData.size(); ++i)
    {
        const SpriteData &memSpriteData = levelManager.getPictureData().
                getSpriteData()[enemiesData[i].m_staticSprites[0]];
        for(uint32_t j = 0; j < enemiesData[i].m_TileGamePosition.size(); ++j)
        {
            uint32_t numEntity = createEnemyEntity();
            confBaseComponent(numEntity, memSpriteData,
                              enemiesData[i].m_TileGamePosition[j], CollisionShape_e::CIRCLE_C);
            loadEnemySprites(levelManager.getPictureData().getSpriteData(), enemiesData, numEntity);
        }
    }
}

//===================================================================
void MainEngine::loadEnemySprites(const std::vector<SpriteData> &vectSprite,
                                  const std::vector<EnemyData> &enemiesData, uint32_t numEntity)
{
    MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(numEntity, Components_e::MEM_SPRITE_DATA_COMPONENT);
    assert(memSpriteComp);
    for(uint32_t i = 0; i < enemiesData.size(); ++i)
    {
        uint32_t vectSize = static_cast<uint32_t>(EnemySpriteType_e::TOTAL_SPRITE);
        memSpriteComp->m_vectSpriteData.reserve(vectSize);
        for(uint32_t j = 0; j < enemiesData[i].m_staticSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_staticSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_moveSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_moveSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_attackSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_attackSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_dyingSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_dyingSprites[j]]);
        }
        //OOOOOK TMP
        assert(memSpriteComp->m_vectSpriteData.size() > vectSize);
    }
}


//===================================================================
uint32_t MainEngine::createWallEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::RECTANGLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
//    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createDoorEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::RECTANGLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::DOOR_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createEnemyEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::MOVEABLE_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createStaticEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::STATIC_ELEMENT_COMPONENT] = true;
    bitsetComponents[Components_e::RECTANGLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
void MainEngine::confBaseComponent(uint32_t entityNum,
                                      const SpriteData &memSpriteData,
                                      const pairUI_t& coordLevel,
                                      CollisionShape_e collisionShape)
{
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(entityNum, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    spriteComp->m_spriteData = &memSpriteData;
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
            searchComponentByType<MapCoordComponent>(entityNum, Components_e::MAP_COORD_COMPONENT);
    assert(mapComp);
    GeneralCollisionComponent *tagComp = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(entityNum, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(tagComp);
    if(collisionShape == CollisionShape_e::RECTANGLE_C)
    {
        RectangleCollisionComponent *rectComp = m_ecsManager.getComponentManager().
                searchComponentByType<RectangleCollisionComponent>(entityNum, Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(rectComp);
        rectComp->m_size = {LEVEL_TILE_SIZE_PX, LEVEL_TILE_SIZE_PX};
        tagComp->m_tag = CollisionTag_e::WALL_CT;
    }
    else if(collisionShape == CollisionShape_e::CIRCLE_C)
    {
        CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
                searchComponentByType<CircleCollisionComponent>(entityNum, Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(circleComp);
        circleComp->m_ray = ENEMY_RAY;
        tagComp->m_tag = CollisionTag_e::ENEMY_CT;
        spriteComp->m_glFpsSize.second = 1.4f;
    }
    mapComp->m_coord = coordLevel;
    mapComp->m_absoluteMapPositionPX = getAbsolutePosition(coordLevel);
    tagComp->m_shape = collisionShape;
}

//===================================================================
void MainEngine::confStaticComponent(uint32_t entityNum,
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
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::MOVEABLE_COMPONENT] = true;
    bitsetComponents[Components_e::COLOR_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::INPUT_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::VISION_COMPONENT] = true;
    bitsetComponents[Components_e::PLAYER_CONF_COMPONENT] = true;
    uint32_t entityNum = m_ecsManager.addEntity(bitsetComponents);
    confPlayerEntity(entityNum, level);
    //notify player entity number
    m_graphicEngine.getMapDisplaySystem().confPlayerComp(entityNum);
}

//===================================================================
void MainEngine::confPlayerEntity(uint32_t entityNum, const Level &level)
{
    PositionVertexComponent *pos = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    MapCoordComponent *map = m_ecsManager.getComponentManager().
            searchComponentByType<MapCoordComponent>(entityNum,
                                                     Components_e::MAP_COORD_COMPONENT);
    MoveableComponent *move = m_ecsManager.getComponentManager().
            searchComponentByType<MoveableComponent>(entityNum,
                                                     Components_e::MOVEABLE_COMPONENT);
    ColorVertexComponent *color = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent>(entityNum,
                                                     Components_e::COLOR_VERTEX_COMPONENT);
    CircleCollisionComponent *circleColl = m_ecsManager.getComponentManager().
            searchComponentByType<CircleCollisionComponent>(entityNum,
                                                     Components_e::CIRCLE_COLLISION_COMPONENT);
    GeneralCollisionComponent *tagColl = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(entityNum,
                                                     Components_e::GENERAL_COLLISION_COMPONENT);
    VisionComponent *vision = m_ecsManager.getComponentManager().
            searchComponentByType<VisionComponent>(entityNum,
                                                     Components_e::VISION_COMPONENT);
    assert(pos);
    assert(map);
    assert(move);
    assert(color);
    assert(circleColl);
    assert(tagColl);
    assert(vision);
//    vision->m_coneVision = 90.0f;

    map->m_coord = level.getPlayerDeparture();
    Direction_e playerDir = level.getPlayerDepartureDirection();
    switch(playerDir)
    {
    case Direction_e::NORTH:
        move->m_degreeOrientation = 90.0f;
        break;
    case Direction_e::EAST:
        move->m_degreeOrientation = 0.0f;
        break;
    case Direction_e::SOUTH:
        move->m_degreeOrientation = 270.0f;
        break;
    case Direction_e::WEST:
        move->m_degreeOrientation = 180.0f;
        break;
    }
    map->m_absoluteMapPositionPX = getAbsolutePosition(map->m_coord);
    updatePlayerOrientation(*move, *pos, *vision);

    color->m_vertex.reserve(3);
    color->m_vertex.emplace_back(0.9f, 0.00f, 0.00f);
    color->m_vertex.emplace_back(0.9f, 0.00f, 0.00f);
    color->m_vertex.emplace_back(0.9f, 0.00f, 0.00f);
    circleColl->m_ray = PLAYER_RAY;
    tagColl->m_tag = CollisionTag_e::PLAYER_CT;
    tagColl->m_shape = CollisionShape_e::CIRCLE_C;
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
                confBaseComponent(entityNum,
                                     memSpriteData,
                                     staticData->operator[](i).m_TileGamePosition[j],
                        CollisionShape_e::RECTANGLE_C);
                confStaticComponent(entityNum,
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
    posComp->m_vertex.reserve(4);
    posComp->m_vertex.emplace_back(-1.0f, 0.0f);
    posComp->m_vertex.emplace_back(1.0f, 0.0f);
    posComp->m_vertex.emplace_back(1.0f, -1.0f);
    posComp->m_vertex.emplace_back(-1.0f, -1.0f);

    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent>(entityNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(colorComp);
    colorComp->m_vertex.reserve(4);
    colorComp->m_vertex.emplace_back(0.1f, 0.2f, 0.1f);
    colorComp->m_vertex.emplace_back(0.1f, 0.2f, 0.1f);
    colorComp->m_vertex.emplace_back(0.3f, 0.9f, 0.1f);
    colorComp->m_vertex.emplace_back(0.3f, 0.9f, 0.1f);

}

//===================================================================
void MainEngine::confCeilingComponents(uint32_t entityNum)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex.reserve(4);
    posComp->m_vertex.emplace_back(-1.0f, 1.0f);
    posComp->m_vertex.emplace_back(1.0f, 1.0f);
    posComp->m_vertex.emplace_back(1.0f, 0.0f);
    posComp->m_vertex.emplace_back(-1.0f, 0.0f);

    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent>(entityNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(colorComp);
    colorComp->m_vertex.reserve(4);
    colorComp->m_vertex.emplace_back(0.1f, 0.2f, 0.3f);
    colorComp->m_vertex.emplace_back(0.1f, 0.2f, 0.3f);
    colorComp->m_vertex.emplace_back(0.3f, 0.9f, 0.8f);
    colorComp->m_vertex.emplace_back(0.3f, 0.9f, 0.8f);
}

//===================================================================
void MainEngine::linkSystemsToGraphicEngine()
{
    ColorDisplaySystem *color = m_ecsManager.getSystemManager().
            searchSystemByType<ColorDisplaySystem>(static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM));
    MapDisplaySystem *map = m_ecsManager.getSystemManager().
            searchSystemByType<MapDisplaySystem>(static_cast<uint32_t>(Systems_e::MAP_DISPLAY_SYSTEM));
    FirstPersonDisplaySystem *first = m_ecsManager.getSystemManager().
            searchSystemByType<FirstPersonDisplaySystem>(static_cast<uint32_t>(Systems_e::FIRST_PERSON_DISPLAY_SYSTEM));
    VisionSystem *vision = m_ecsManager.getSystemManager().
            searchSystemByType<VisionSystem>(static_cast<uint32_t>(Systems_e::VISION_SYSTEM));
    m_graphicEngine.linkSystems(color, map, first, vision);
}

//===================================================================
void MainEngine::linkSystemsToPhysicalEngine()
{
    InputSystem *input = m_ecsManager.getSystemManager().
            searchSystemByType<InputSystem>(static_cast<uint32_t>(Systems_e::INPUT_SYSTEM));
    CollisionSystem *coll = m_ecsManager.getSystemManager().
            searchSystemByType<CollisionSystem>(static_cast<uint32_t>(Systems_e::COLLISION_SYSTEM));
    DoorSystem *door = m_ecsManager.getSystemManager().
            searchSystemByType<DoorSystem>(static_cast<uint32_t>(Systems_e::DOOR_SYSTEM));
    input->setGLWindow(m_graphicEngine.getGLWindow());
    m_physicalEngine.linkSystems(input, coll, door);
}
