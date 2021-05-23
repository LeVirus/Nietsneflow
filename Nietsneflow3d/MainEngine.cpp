#include "MainEngine.hpp"
#include "constants.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/FPSVisibleStaticElementComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/MemPositionsVertexComponents.hpp>
#include <ECS/Components/SegmentCollisionComponent.hpp>
#include <ECS/Components/WriteComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/ShotConfComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <ECS/Systems/CollisionSystem.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <ECS/Systems/VisionSystem.hpp>
#include <ECS/Systems/DoorSystem.hpp>
#include <ECS/Systems/StaticDisplaySystem.hpp>
#include <ECS/Systems/IASystem.hpp>
#include <LevelManager.hpp>
#include <cassert>

//===================================================================
void MainEngine::init()
{
    m_ecsManager.init();
    linkSystemsToGraphicEngine();
    linkSystemsToPhysicalEngine();
}

//===================================================================
bool MainEngine::mainLoop(bool &memGameOver)
{
    memGameOver = false;
    m_graphicEngine.getMapSystem().confLevelData();
    if(m_playerMem)
    {
        loadPlayerGear();
    }
    else
    {
        memPlayerGear();
    }
    m_graphicEngine.unsetTransition(m_gamePaused);
    do
    {
        m_physicalEngine.runIteration(m_gamePaused);
        clearObjectToDelete();
        m_graphicEngine.runIteration(m_gamePaused);
        if(!m_exitColl->m_active)
        {
            memPlayerGear();
            m_graphicEngine.setTransition(m_gamePaused);
            displayTransitionMenu();
            return true;
        }
        if(!m_playerConf->m_life)
        {
            memGameOver = true;
            m_graphicEngine.setTransition(m_gamePaused);
            displayTransitionMenu();
            return true;
        }
    }while(!m_graphicEngine.windowShouldClose());
    return false;
}

//===================================================================
void MainEngine::memPlayerGear()
{
    assert(m_playerConf);
    m_memPlayerConf.m_ammunationsCount = m_playerConf->m_ammunationsCount;
    m_memPlayerConf.m_currentWeapon = m_playerConf->m_currentWeapon;
    m_memPlayerConf.m_previousWeapon = m_playerConf->m_previousWeapon;
    m_memPlayerConf.m_weapons = m_playerConf->m_weapons;
    m_memPlayerConf.m_life = m_playerConf->m_life;
    m_playerMem = true;
}

//===================================================================
void MainEngine::loadPlayerGear()
{
    assert(m_playerConf);
    m_playerConf->m_ammunationsCount = m_memPlayerConf.m_ammunationsCount;
    m_playerConf->m_currentWeapon = m_memPlayerConf.m_currentWeapon;
    m_playerConf->m_previousWeapon = m_memPlayerConf.m_previousWeapon;
    m_playerConf->m_weapons = m_memPlayerConf.m_weapons;
    m_playerConf->m_life = m_memPlayerConf.m_life;
    m_playerMem = false;
    StaticDisplaySystem *staticDisplay = m_ecsManager.getSystemManager().
            searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    assert(staticDisplay);
    //update FPS weapon sprite
    std::map<WeaponsType_e, WeaponsSpriteType_e>::const_iterator it =
            staticDisplay->getWeaponsSpriteAssociated().find(m_playerConf->m_currentWeapon);
    assert(it != staticDisplay->getWeaponsSpriteAssociated().end());
    staticDisplay->setWeaponSprite(m_playerConf->m_weaponEntity, it->second);
}

//===================================================================
void MainEngine::displayTransitionMenu()
{
    m_gamePaused = true;
    assert(m_writeConf);
    m_graphicEngine.fillMenuWrite(m_writeConf, "CONTINUE");
    m_physicalEngine.setModeTransitionMenu(true);
    m_graphicEngine.mainDisplay(m_gamePaused);
    m_playerConf->m_currentCursorPos = static_cast<CurrentMenuCursorPos_e>(0);
    m_graphicEngine.unsetTransition(m_gamePaused);
    do
    {
        m_graphicEngine.runIteration(m_gamePaused);
        m_physicalEngine.runIteration(m_gamePaused);
    }while(m_gamePaused);
    m_physicalEngine.setModeTransitionMenu(false);
    m_graphicEngine.setTransition(true);
    m_graphicEngine.fillMenuWrite(m_writeConf, MENU_ENTRIES);
}

//===================================================================
void MainEngine::confPlayerShoot(const AmmoContainer_t &playerVisibleShots,
                                 const pairFloat_t &point, float degreeAngle)
{
    m_physicalEngine.confVisibleShoot(playerVisibleShots, point, degreeAngle);
}

//===================================================================
void MainEngine::playerShoot(PlayerConfComponent *playerComp, const pairFloat_t &point,
                             float degreeAngle)
{
    if(playerComp->m_currentWeapon == WeaponsType_e::TOTAL)
    {
        return;
    }
    GeneralCollisionComponent *genColl = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(*playerComp->m_shootEntities[0],
            Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genColl);
    if(playerComp->m_currentWeapon == WeaponsType_e::GUN)
    {
        SegmentCollisionComponent *segmentColl = m_ecsManager.getComponentManager().
                searchComponentByType<SegmentCollisionComponent>(*playerComp->m_shootEntities[0],
                Components_e::SEGMENT_COLLISION_COMPONENT);
        assert(segmentColl);
        confBullet(genColl, segmentColl, CollisionTag_e::BULLET_PLAYER_CT, point, degreeAngle);
    }
    else if(playerComp->m_currentWeapon == WeaponsType_e::SHOTGUN)
    {
        confPlayerShoot(playerComp->m_visibleShootEntities, point, degreeAngle);
    }
    uint32_t currentWeapon = static_cast<uint32_t>(playerComp->m_currentWeapon);
    assert(playerComp->m_ammunationsCount[currentWeapon] > 0);
    --playerComp->m_ammunationsCount[currentWeapon];
}

//===================================================================
void confBullet(GeneralCollisionComponent *genColl, SegmentCollisionComponent *segmentColl,
                CollisionTag_e collTag, const pairFloat_t &point, float degreeAngle)
{
    assert(collTag == CollisionTag_e::BULLET_ENEMY_CT ||
           collTag == CollisionTag_e::BULLET_PLAYER_CT);
    genColl->m_tag = collTag;
    genColl->m_shape = CollisionShape_e::SEGMENT_C;
    genColl->m_active = true;
    segmentColl->m_degreeOrientation = degreeAngle;
    segmentColl->m_points.first = point;
}

//===================================================================
void MainEngine::setUnsetPaused()
{
    m_gamePaused = !m_gamePaused;
    if(m_gamePaused)
    {
        memTimerPausedValue();
    }
    else
    {
        applyTimerPausedValue();
    }
}

//===================================================================
void MainEngine::confSystems()
{
    m_graphicEngine.confSystems();
}

//===================================================================
void MainEngine::clearObjectToDelete()
{
    const std::vector<uint32_t> &vect = m_physicalEngine.getObjectEntityToDelete();
    if(vect.empty())
    {
        return;
    }
    for(uint32_t i = 0; i < vect.size(); ++i)
    {
        m_ecsManager.bRmEntity(vect[i]);
    }
    m_physicalEngine.clearVectObjectToDelete();
}

//===================================================================
void MainEngine::memColorSystemBackgroundEntities(uint32_t ground, uint32_t ceiling)
{
    m_graphicEngine.memColorSystemBackgroundEntities(ground, ceiling);
}

//===================================================================
void MainEngine::memTimerPausedValue()
{
    TimerComponent *timerComp;
    std::bitset<Components_e::TOTAL_COMPONENTS> bitset;
    bitset[Components_e::TIMER_COMPONENT] = true;
    std::vector<uint32_t> vectEntities = m_ecsManager.getEntityContainingComponents(bitset);
    assert(m_vectMemPausedTimer.empty());
    m_vectMemPausedTimer.reserve(vectEntities.size());
    for(uint32_t i = 0; i < vectEntities.size(); ++i)
    {
        timerComp = m_ecsManager.getComponentManager().
                searchComponentByType<TimerComponent>(vectEntities[i],
                                                      Components_e::TIMER_COMPONENT);
        assert(timerComp);
        time_t time = (std::chrono::system_clock::to_time_t(
                           std::chrono::system_clock::now()) -
                       std::chrono::system_clock::to_time_t(timerComp->m_clockA));
        m_vectMemPausedTimer.emplace_back(vectEntities[i], time);
    }
}

//===================================================================
void MainEngine::applyTimerPausedValue()
{
    TimerComponent *timerComp;
    for(uint32_t i = 0; i < m_vectMemPausedTimer.size(); ++i)
    {
        timerComp = m_ecsManager.getComponentManager().
                searchComponentByType<TimerComponent>(m_vectMemPausedTimer[i].first,
                                                      Components_e::TIMER_COMPONENT);
        assert(timerComp);
        timerComp->m_clockA = std::chrono::system_clock::from_time_t( std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now()) - m_vectMemPausedTimer[i].second);
    }
    m_vectMemPausedTimer.clear();
}

//===================================================================
void MainEngine::loadDamageEntity()
{
    uint32_t entityNum = createColorEntity();
    confUnifiedColorEntity(entityNum, {0.7f, 0.2f, 0.1f});
    m_ecsManager.getSystemManager().searchSystemByType<ColorDisplaySystem>(
                static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->loadDamageEntity(entityNum);}

//===================================================================
void MainEngine::loadTransitionEntity()
{
    uint32_t entityNum = createColorEntity();
    confUnifiedColorEntity(entityNum, {0.0f, 0.0f, 0.0f});
    m_ecsManager.getSystemManager().searchSystemByType<ColorDisplaySystem>(
                static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->loadTransitionEntity(entityNum);
}

//===================================================================
void MainEngine::confUnifiedColorEntity(uint32_t entityNum, const tupleFloat_t &color)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    if(!posComp->m_vertex.empty())
    {
        posComp->m_vertex.clear();
    }
    posComp->m_vertex.reserve(4);
    posComp->m_vertex.emplace_back(-1.0f, 1.0f);
    posComp->m_vertex.emplace_back(1.0f, 1.0f);
    posComp->m_vertex.emplace_back(1.0f, -1.0f);
    posComp->m_vertex.emplace_back(-1.0f, -1.0f);
    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent>(entityNum, Components_e::COLOR_VERTEX_COMPONENT);
    assert(colorComp);
    if(!colorComp->m_vertex.empty())
    {
        colorComp->m_vertex.clear();
    }
    colorComp->m_vertex.reserve(4);
    colorComp->m_vertex.emplace_back(std::get<0>(color), std::get<1>(color),
                                     std::get<2>(color), 1.0f);
    colorComp->m_vertex.emplace_back(std::get<0>(color), std::get<1>(color),
                                     std::get<2>(color), 1.0f);
    colorComp->m_vertex.emplace_back(std::get<0>(color), std::get<1>(color),
                                     std::get<2>(color), 1.0f);
    colorComp->m_vertex.emplace_back(std::get<0>(color), std::get<1>(color),
                                     std::get<2>(color), 1.0f);
}

//===================================================================
uint32_t MainEngine::createColorEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::COLOR_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
void MainEngine::loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                              const GroundCeilingData &ceilingData)
{
    uint32_t ceiling, ground;
    ground = createBackgroundEntity(&groundData);
    confGroundComponents(ground);
    ceiling = createBackgroundEntity(&ceilingData);
    confCeilingComponents(ceiling);
    memColorSystemBackgroundEntities(ground, ceiling);
}

//===================================================================
uint32_t MainEngine::createBackgroundEntity(GroundCeilingData const *data)
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    if(data->m_apparence == DisplayType_e::COLOR)
    {
        bitsetComponents[Components_e::COLOR_VERTEX_COMPONENT] = true;
    }
    else if(data->m_apparence == DisplayType_e::TEXTURE)
    {
        bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    }
    else
    {
        bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
        bitsetComponents[Components_e::COLOR_VERTEX_COMPONENT] = true;
    }
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
void MainEngine::loadGraphicPicture(const PictureData &picData, const FontData &fontData)
{
    m_graphicEngine.loadPictureData(picData, fontData);
}

//===================================================================
void MainEngine::loadLevelEntities(const LevelManager &levelManager)
{
    loadGroundAndCeilingEntities(levelManager.getPictureData().getGroundData(),
                                 levelManager.getPictureData().getCeilingData());
    loadDamageEntity();
    loadTransitionEntity();
    loadStaticElementEntities(levelManager);
    loadPlayerEntity(levelManager.getPictureData().getSpriteData(),
                     levelManager.getLevel(), loadWeaponsEntity(levelManager));
    Level::initLevelElementArray();
    loadWallEntities(levelManager);
    loadDoorEntities(levelManager);
    loadEnemiesEntities(levelManager);
}

//===================================================================
uint32_t MainEngine::loadWeaponsEntity(const LevelManager &levelManager)
{
    uint32_t weaponEntity = loadWeaponEntity();
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    const std::vector<pairUIPairFloat_t> &vectWeapons = levelManager.
            getLevel().getWeaponsData();
    MemSpriteDataComponent *memSprite = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(weaponEntity,
                                                          Components_e::MEM_SPRITE_DATA_COMPONENT);
    MemPositionsVertexComponents *memPosVertex = m_ecsManager.getComponentManager().
            searchComponentByType<MemPositionsVertexComponents>(weaponEntity,
                                                                Components_e::MEM_POSITIONS_VERTEX_COMPONENT);
    assert(memSprite);
    assert(memPosVertex);
    memSprite->m_vectSpriteData.reserve(static_cast<uint32_t>(WeaponsSpriteType_e::TOTAL_SPRITE));
    float posUp, posDown = -1.0f, posLeft, posRight, diffLateral;
    for(uint32_t i = 0; i < vectWeapons.size(); ++i)
    {
        memSprite->m_vectSpriteData.emplace_back(&vectSprite[vectWeapons[i].first]);
        posUp = -1.0f + vectWeapons[i].second.second;
        diffLateral = vectWeapons[i].second.first / 2.0f;
        posLeft = -diffLateral;
        posRight = diffLateral;
        memPosVertex->m_vectSpriteData.emplace_back(std::array<pairFloat_t, 4>{
                                                        {
                                                            {posLeft, posUp},
                                                            {posRight, posUp},
                                                            {posRight, posDown},
                                                            {posLeft, posDown}
                                                        }
                                                    });
    }
    return weaponEntity;
}

//===================================================================
void MainEngine::loadWallEntities(const LevelManager &levelManager)
{
    const std::vector<WallData> &wallData = levelManager.getLevel().getWallData();
    MemSpriteDataComponent *memSpriteComp;
    SpriteTextureComponent *spriteComp;
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    assert(!Level::getLevelCaseType().empty());
    for(uint32_t i = 0; i < wallData.size(); ++i)
    {
        const SpriteData &memSpriteData = levelManager.getPictureData().getSpriteData()[wallData[i].m_sprites[0]];
        for(uint32_t j = 0; j < wallData[i].m_TileGamePosition.size(); ++j)
        {
            uint32_t numEntity = createWallEntity();
            confBaseComponent(numEntity, memSpriteData, wallData[i].m_TileGamePosition[j],
                              CollisionShape_e::RECTANGLE_C, CollisionTag_e::WALL_CT);
            spriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
            assert(spriteComp);
            Level::addElementCase(spriteComp, wallData[i].m_TileGamePosition[j], LevelCaseType_e::WALL_LC, numEntity);
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
                              CollisionShape_e::RECTANGLE_C, CollisionTag_e::DOOR_CT);
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

            SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
            assert(spriteComp);
            Level::addElementCase(spriteComp, doorData[i].m_TileGamePosition[j], LevelCaseType_e::DOOR_LC, numEntity);

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
                getSpriteData()[enemiesData[i].m_staticFrontSprites[0]];
        for(uint32_t j = 0; j < enemiesData[i].m_TileGamePosition.size(); ++j)
        {
            uint32_t numEntity = createEnemyEntity();
            confBaseComponent(numEntity, memSpriteData, enemiesData[i].m_TileGamePosition[j],
                              CollisionShape_e::CIRCLE_C, CollisionTag_e::ENEMY_CT);
            EnemyConfComponent *enemyComp = m_ecsManager.getComponentManager().
                    searchComponentByType<EnemyConfComponent>(numEntity,
                                                              Components_e::ENEMY_CONF_COMPONENT);
            FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
                    searchComponentByType<FPSVisibleStaticElementComponent>(
                        numEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
            assert(enemyComp);
            assert(fpsStaticComp);
            fpsStaticComp->m_inGameSpriteSize = enemiesData[i].m_inGameSpriteSize;
            createAmmosEntities(enemyComp->m_stdAmmo, CollisionTag_e::BULLET_ENEMY_CT);
            createAmmosEntities(enemyComp->m_visibleAmmo, CollisionTag_e::BULLET_ENEMY_CT, true);
            loadEnemySprites(levelManager.getPictureData().getSpriteData(),
                             enemiesData, numEntity, enemyComp->m_visibleAmmo);
            MoveableComponent *moveComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MoveableComponent>(numEntity,
                                                              Components_e::MOVEABLE_COMPONENT);
            assert(moveComp);
            moveComp->m_velocity = 1.0f;
        }
    }
}

//===================================================================
void MainEngine::createAmmosEntities(AmmoContainer_t &ammoCount, CollisionTag_e collTag,
                                     bool visibleShot)
{
    for(uint32_t i = 0; i < ammoCount.size(); ++i)
    {
        if(!visibleShot)
        {
            ammoCount[i] = createShotEntity();
        }
        else
        {
            ammoCount[i] = createVisibleShotEntity();
        }
        GeneralCollisionComponent *genColl = m_ecsManager.getComponentManager().
                searchComponentByType<GeneralCollisionComponent>(*ammoCount[i],
                                                                 Components_e::GENERAL_COLLISION_COMPONENT);
        assert(genColl);
        genColl->m_active = false;
        genColl->m_tag = collTag;
        genColl->m_shape = (visibleShot) ? CollisionShape_e::CIRCLE_C : CollisionShape_e::SEGMENT_C;
    }
    if(visibleShot)
    {
        confVisibleAmmo(ammoCount);
    }
}

//===================================================================
void MainEngine::loadEnemySprites(const std::vector<SpriteData> &vectSprite,
                                  const std::vector<EnemyData> &enemiesData, uint32_t numEntity,
                                  const AmmoContainer_t &visibleAmmo)
{
    MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(numEntity,
                                                          Components_e::MEM_SPRITE_DATA_COMPONENT);
    assert(memSpriteComp);
    for(uint32_t i = 0; i < enemiesData.size(); ++i)
    {
        uint32_t vectSize = static_cast<uint32_t>(EnemySpriteType_e::TOTAL_SPRITE);
        memSpriteComp->m_vectSpriteData.reserve(vectSize);
        for(uint32_t j = 0; j < enemiesData[i].m_staticFrontSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_staticFrontSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_staticFrontLeftSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_staticFrontLeftSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_staticFrontRightSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_staticFrontRightSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_staticBackSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_staticBackSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_staticBackLeftSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_staticBackLeftSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_staticBackRightSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_staticBackRightSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_staticLeftSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_staticLeftSprites[j]]);
        }
        for(uint32_t j = 0; j < enemiesData[i].m_staticRightSprites.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemiesData[i].m_staticRightSprites[j]]);
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
        if(!enemiesData[i].m_visibleShotSprites.empty())
        {
            loadVisibleShotEnemySprites(vectSprite, visibleAmmo, enemiesData[i]);
        }
        assert(memSpriteComp->m_vectSpriteData.size() == vectSize);
    }
}

//===================================================================
void MainEngine::loadVisibleShotEnemySprites(const std::vector<SpriteData> &vectSprite,
                                             const AmmoContainer_t &visibleAmmo,
                                             const EnemyData &enemyData)
{
    for(uint32_t k = 0; k < visibleAmmo.size(); ++k)
    {
        SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                searchComponentByType<SpriteTextureComponent>(*visibleAmmo[k],
                                                              Components_e::SPRITE_TEXTURE_COMPONENT);
        MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().
                searchComponentByType<MemSpriteDataComponent>(*visibleAmmo[k],
                                                              Components_e::MEM_SPRITE_DATA_COMPONENT);
        ShotConfComponent *shotConfComp = m_ecsManager.getComponentManager().
                searchComponentByType<ShotConfComponent>(*visibleAmmo[k],
                                                              Components_e::SHOT_CONF_COMPONENT);
        assert(shotConfComp);
        assert(spriteComp);
        assert(memSpriteComp);
        memSpriteComp->m_vectSpriteData.reserve(enemyData.m_visibleShotSprites.size());
        for(uint32_t l = 0; l < enemyData.m_visibleShotSprites.size(); ++l)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemyData.m_visibleShotSprites[l]]);
        }
        for(uint32_t l = 0; l < enemyData.m_visibleShotDestructSprites.size(); ++l)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[enemyData.m_visibleShotDestructSprites[l]]);
        }
        spriteComp->m_spriteData = &vectSprite[enemyData.
                m_visibleShotSprites[static_cast<uint32_t>(shotConfComp->m_spritePhaseShot)]];
    }
}

//===================================================================
void MainEngine::confVisibleAmmo(const AmmoContainer_t &ammoCont)
{
    for(uint32_t i = 0; i < ammoCont.size(); ++i)
    {
        CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
                searchComponentByType<CircleCollisionComponent>(*ammoCont[i],
                                                                Components_e::CIRCLE_COLLISION_COMPONENT);
        FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
                searchComponentByType<FPSVisibleStaticElementComponent>(
                    *ammoCont[i], Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
        MoveableComponent *moveComp = m_ecsManager.getComponentManager().
                searchComponentByType<MoveableComponent>(
                    *ammoCont[i], Components_e::MOVEABLE_COMPONENT);
        assert(circleComp);
        assert(fpsStaticComp);
        assert(moveComp);
        circleComp->m_ray = 5.0f;
        fpsStaticComp->m_inGameSpriteSize = {0.2f, 0.3f};
        moveComp->m_velocity = 5.0f;
    }
}

//===================================================================
uint32_t MainEngine::loadWeaponEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_POSITIONS_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
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
    bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::MOVEABLE_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComponents[Components_e::ENEMY_CONF_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createShotEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::SEGMENT_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::SHOT_CONF_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createVisibleShotEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MOVEABLE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    bitsetComponents[Components_e::SHOT_CONF_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createWriteEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::WRITE_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createSimpleSpriteEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createStaticEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createObjectEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::OBJECT_CONF_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
void MainEngine::confBaseComponent(uint32_t entityNum, const SpriteData &memSpriteData,
                                   const pairUI_t& coordLevel, CollisionShape_e collisionShape,
                                   CollisionTag_e tag)
{
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(entityNum, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    spriteComp->m_spriteData = &memSpriteData;
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
            searchComponentByType<MapCoordComponent>(entityNum, Components_e::MAP_COORD_COMPONENT);
    assert(mapComp);
    mapComp->m_coord = coordLevel;
    mapComp->m_absoluteMapPositionPX = getAbsolutePosition(coordLevel);
    GeneralCollisionComponent *tagComp = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(entityNum, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(tagComp);
    tagComp->m_shape = collisionShape;
    if(collisionShape == CollisionShape_e::RECTANGLE_C)
    {
        RectangleCollisionComponent *rectComp = m_ecsManager.getComponentManager().
                searchComponentByType<RectangleCollisionComponent>(entityNum, Components_e::RECTANGLE_COLLISION_COMPONENT);
        assert(rectComp);
        rectComp->m_size = {LEVEL_TILE_SIZE_PX, LEVEL_TILE_SIZE_PX};
    }
    else if(collisionShape == CollisionShape_e::CIRCLE_C)
    {
        FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
                searchComponentByType<FPSVisibleStaticElementComponent>(
                    entityNum, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
        CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
                searchComponentByType<CircleCollisionComponent>(entityNum, Components_e::CIRCLE_COLLISION_COMPONENT);
        assert(fpsStaticComp);
        assert(circleComp);
        circleComp->m_ray = ENEMY_RAY;
        fpsStaticComp->m_inGameSpriteSize = {0.5f, 0.8f};
    }
    tagComp->m_tag = tag;
}

//===================================================================
void MainEngine::confStaticComponent(uint32_t entityNum, const pairFloat_t& elementSize,
                                     LevelStaticElementType_e elementType)
{
    FPSVisibleStaticElementComponent *staticComp = m_ecsManager.getComponentManager().
            searchComponentByType<FPSVisibleStaticElementComponent>(entityNum, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(staticComp);
    staticComp->m_inGameSpriteSize = elementSize;
    staticComp->m_levelElementType = elementType;
}

//===================================================================
void MainEngine::loadPlayerEntity(const std::vector<SpriteData> &vectSpriteData,
                                  const Level &level, uint32_t numWeaponEntity)
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
    confPlayerEntity(vectSpriteData, entityNum, level, numWeaponEntity);
    //notify player entity number
    m_graphicEngine.getMapSystem().confPlayerComp(entityNum);
    m_physicalEngine.memPlayerEntity(entityNum);
}

//===================================================================
void MainEngine::confPlayerEntity(const std::vector<SpriteData> &vectSpriteData,
                                  uint32_t entityNum, const Level &level, uint32_t numWeaponEntity)
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
    PlayerConfComponent *playerConf = m_ecsManager.getComponentManager().
            searchComponentByType<PlayerConfComponent>(entityNum,
                                                     Components_e::PLAYER_CONF_COMPONENT);
    assert(pos);
    assert(map);
    assert(move);
    assert(color);
    assert(circleColl);
    assert(tagColl);
    assert(vision);
    assert(playerConf);
    m_playerConf = playerConf;
    createAmmosEntities(playerConf->m_shootEntities, CollisionTag_e::BULLET_PLAYER_CT);
    createAmmosEntities(playerConf->m_visibleShootEntities, CollisionTag_e::BULLET_PLAYER_CT, true);
    confShotsEntities(playerConf->m_shootEntities, 1);
    confShotsEntities(playerConf->m_visibleShootEntities, 1);
    loadPlayerVisibleShotsSprite(vectSpriteData, level.getVisibleShotsData(),
                                 playerConf->m_visibleShootEntities);
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
    color->m_vertex.emplace_back(0.9f, 0.00f, 0.00f, 1.0f);
    color->m_vertex.emplace_back(0.9f, 0.00f, 0.00f, 1.0f);
    color->m_vertex.emplace_back(0.9f, 0.00f, 0.00f, 1.0f);
    circleColl->m_ray = PLAYER_RAY;
    tagColl->m_tag = CollisionTag_e::PLAYER_CT;
    tagColl->m_shape = CollisionShape_e::CIRCLE_C;
    playerConf->m_weaponEntity = numWeaponEntity;
    playerConf->m_currentWeapon = WeaponsType_e::GUN;
    playerConf->m_previousWeapon = WeaponsType_e::GUN;
    //set standart weapon sprite
    StaticDisplaySystem *staticDisplay = m_ecsManager.getSystemManager().
            searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    assert(staticDisplay);
    staticDisplay->setWeaponSprite(numWeaponEntity, WeaponsSpriteType_e::GUN_STATIC);
    confWriteEntities(playerConf);
    confMenuCursorEntity(playerConf);
}

//===================================================================
void MainEngine::confShotsEntities(const AmmoContainer_t &ammoEntities, uint32_t damageValue)
{
    for(uint32_t i = 0; i < ammoEntities.size(); ++i)
    {
        ShotConfComponent *shotConfComp = m_ecsManager.getComponentManager().
                searchComponentByType<ShotConfComponent>(*ammoEntities[i],
                                                         Components_e::SHOT_CONF_COMPONENT);
        assert(shotConfComp);
        shotConfComp->m_damage = damageValue;
    }
}

//===================================================================
void MainEngine::loadPlayerVisibleShotsSprite(const std::vector<SpriteData> &vectSpriteData,
                                              const std::vector<uint8_t> &vectSprite,
                                              const AmmoContainer_t &ammoEntities)
{
    for(uint32_t k = 0; k < ammoEntities.size(); ++k)
    {
        MemSpriteDataComponent *memComp = m_ecsManager.getComponentManager().
                searchComponentByType<MemSpriteDataComponent>(*ammoEntities[k],
                                                              Components_e::MEM_SPRITE_DATA_COMPONENT);
        SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                searchComponentByType<SpriteTextureComponent>(*ammoEntities[k],
                                                              Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(memComp);
        assert(spriteComp);
        assert(vectSprite.size() == static_cast<uint32_t>(ShotPhase_e::TOTAL));
        for(uint32_t l = 0; l < vectSprite.size(); ++l)
        {
            memComp->m_vectSpriteData.emplace_back(&vectSpriteData[vectSprite[l]]);
        }
        spriteComp->m_spriteData = &vectSpriteData[vectSprite[0]];
    }
}

//===================================================================
void MainEngine::confWriteEntities(PlayerConfComponent *playerConf)
{
    uint32_t numAmmoWrite = createWriteEntity(),
            numLifeWrite = createWriteEntity(), numMenuWrite = createWriteEntity();
    //AMMO
    WriteComponent *writeConf = m_ecsManager.getComponentManager().
            searchComponentByType<WriteComponent>(numAmmoWrite, Components_e::WRITE_COMPONENT);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.95f, -0.9f};
    writeConf->m_fontSize = STD_FONT_SIZE;
    m_graphicEngine.updateAmmoCount(writeConf, playerConf);
    //LIFE
    writeConf = m_ecsManager.getComponentManager().
            searchComponentByType<WriteComponent>(numLifeWrite, Components_e::WRITE_COMPONENT);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.95f, -0.8f};
    writeConf->m_fontSize = STD_FONT_SIZE;
    m_graphicEngine.updatePlayerLife(writeConf, playerConf);
    //MENU
    writeConf = m_ecsManager.getComponentManager().
            searchComponentByType<WriteComponent>(numMenuWrite, Components_e::WRITE_COMPONENT);
    assert(writeConf);
    m_writeConf = writeConf;
    writeConf->m_upLeftPositionGL = m_menuCornerUpLeft;
    writeConf->m_fontSize = MENU_FONT_SIZE;
    m_graphicEngine.fillMenuWrite(writeConf, MENU_ENTRIES);
    playerConf->m_menuEntity = numMenuWrite;
    playerConf->m_ammoWriteEntity = numAmmoWrite;
    playerConf->m_lifeWriteEntity = numLifeWrite;
}

//===================================================================
void MainEngine::confMenuCursorEntity(PlayerConfComponent *playerConf)
{
    uint32_t cursorEntity = createSimpleSpriteEntity();
    PositionVertexComponent *posCursor = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(cursorEntity,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteCursor = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(cursorEntity,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(posCursor);
    assert(spriteCursor);
    assert(m_memCursorSpriteData);
    playerConf->m_menuCursorEntity = cursorEntity;
    spriteCursor->m_spriteData = m_memCursorSpriteData;
    posCursor->m_vertex.reserve(4);
    float leftPos = m_menuCornerUpLeft.first - 0.25f,
            rightPos = m_menuCornerUpLeft.first - 0.05f,
            upPos = m_menuCornerUpLeft.second,
            downPos = m_menuCornerUpLeft.second - 0.25f;
    posCursor->m_vertex.emplace_back(pairFloat_t{leftPos, upPos});
    posCursor->m_vertex.emplace_back(pairFloat_t{rightPos, upPos});
    posCursor->m_vertex.emplace_back(pairFloat_t{rightPos, downPos});
    posCursor->m_vertex.emplace_back(pairFloat_t{leftPos, downPos});
}

//===================================================================
void MainEngine::loadStaticElementEntities(const LevelManager &levelManager)
{
    //LOAD CURSOR MENU
    uint8_t cursorSpriteId = *levelManager.getPictureData().
            getIdentifier(levelManager.getCursorSpriteName());
    m_memCursorSpriteData = &levelManager.getPictureData().getSpriteData()[cursorSpriteId];
    loadStaticElementGroup(levelManager, &levelManager.getLevel().getGroundElementData(),
                           LevelStaticElementType_e::GROUND);
    loadStaticElementGroup(levelManager, &levelManager.getLevel().getCeilingElementData(),
                           LevelStaticElementType_e::CEILING);
    loadStaticElementGroup(levelManager, &levelManager.getLevel().getObjectElementData(),
                           LevelStaticElementType_e::OBJECT);
    loadExitElement(levelManager, levelManager.getLevel().getExitElementData());
}

//===================================================================
void MainEngine::loadExitElement(const LevelManager &levelManager,
                                 const StaticLevelElementData &exit)
{
    const SpriteData &memSpriteData = levelManager.getPictureData().
            getSpriteData()[exit.m_numSprite];
    uint32_t entityNum = createStaticEntity();
    FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
            searchComponentByType<FPSVisibleStaticElementComponent>(
                entityNum, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(fpsStaticComp);
    fpsStaticComp->m_inGameSpriteSize = exit.m_inGameSpriteSize;
    assert(!exit.m_TileGamePosition.empty());
    //mem exit comp determine when level is over
    m_exitColl = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(entityNum, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(m_exitColl);
    confBaseComponent(entityNum, memSpriteData, exit.m_TileGamePosition[0],
            CollisionShape_e::CIRCLE_C, CollisionTag_e::EXIT_CT);
    confStaticComponent(entityNum, exit.m_inGameSpriteSize, LevelStaticElementType_e::GROUND);
}

//===================================================================
void MainEngine::loadStaticElementGroup(const LevelManager &levelManager,
                                        const std::vector<StaticLevelElementData> *staticData,
                                        LevelStaticElementType_e elementType)
{
    CollisionTag_e tag;
    for(uint32_t i = 0; i < staticData->size(); ++i)
    {
        const SpriteData &memSpriteData = levelManager.getPictureData().
                getSpriteData()[staticData->operator[](i).m_numSprite];
        for(uint32_t j = 0; j < staticData->operator[](i).m_TileGamePosition.size(); ++j)
        {
            uint32_t entityNum;
            if(elementType == LevelStaticElementType_e::OBJECT)
            {
                tag = CollisionTag_e::OBJECT_CT;
                entityNum = createObjectEntity();
                ObjectConfComponent *objComp = m_ecsManager.getComponentManager().
                        searchComponentByType<ObjectConfComponent>(entityNum,
                                                                   Components_e::OBJECT_CONF_COMPONENT);
                assert(objComp);
                objComp->m_containing = staticData->operator[](i).m_containing;
                objComp->m_type = staticData->operator[](i).m_type;
            }
            else
            {
                if(staticData->operator[](i).m_traversable)
                {
                    tag = CollisionTag_e::GHOST_CT;
                }
                else
                {
                    tag = CollisionTag_e::WALL_CT;
                }
                entityNum = createStaticEntity();
            }
            FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
                    searchComponentByType<FPSVisibleStaticElementComponent>(
                        entityNum, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
            assert(fpsStaticComp);
            fpsStaticComp->m_inGameSpriteSize = staticData->operator[](i).m_inGameSpriteSize;
            confBaseComponent(entityNum, memSpriteData, staticData->operator[](i).m_TileGamePosition[j],
                    CollisionShape_e::CIRCLE_C, tag);
            confStaticComponent(entityNum, staticData->operator[](i).m_inGameSpriteSize, elementType);
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
    colorComp->m_vertex.emplace_back(0.1f, 0.2f, 0.1f, 1.0f);
    colorComp->m_vertex.emplace_back(0.1f, 0.2f, 0.1f, 1.0f);
    colorComp->m_vertex.emplace_back(0.3f, 0.9f, 0.1f, 1.0f);
    colorComp->m_vertex.emplace_back(0.3f, 0.9f, 0.1f, 1.0f);

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
    colorComp->m_vertex.emplace_back(0.1f, 0.2f, 0.3f, 1.0f);
    colorComp->m_vertex.emplace_back(0.1f, 0.2f, 0.3f, 1.0f);
    colorComp->m_vertex.emplace_back(0.3f, 0.9f, 0.8f, 1.0f);
    colorComp->m_vertex.emplace_back(0.3f, 0.9f, 0.8f, 1.0f);
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
    StaticDisplaySystem *staticDisplay = m_ecsManager.getSystemManager().
            searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    m_graphicEngine.linkSystems(color, map, first, vision, staticDisplay);
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
    IASystem *iaSystem = m_ecsManager.getSystemManager().
            searchSystemByType<IASystem>(static_cast<uint32_t>(Systems_e::IA_SYSTEM));
    input->setGLWindow(m_graphicEngine.getGLWindow());
    input->linkMainEngine(this);
    m_physicalEngine.linkSystems(input, coll, door, iaSystem);
}
