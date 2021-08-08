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
#include <ECS/Components/WeaponComponent.hpp>
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
    std::srand(std::time(nullptr));
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
        savePlayerGear();
    }
    m_graphicEngine.unsetTransition(m_gamePaused);
    do
    {
        m_physicalEngine.runIteration(m_gamePaused);
        clearObjectToDelete();
        m_graphicEngine.runIteration(m_gamePaused);
        if(!m_exitColl->m_active)
        {
            m_playerConf->m_inMovement = false;
            savePlayerGear();
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
void MainEngine::savePlayerGear()
{
    assert(m_playerConf);
    m_memPlayerConf.m_ammunationsCount.resize(m_weaponComp->m_weaponsData.size());
    for(uint32_t i = 0; i < m_memPlayerConf.m_ammunationsCount.size(); ++i)
    {
        m_memPlayerConf.m_ammunationsCount[i] =
                m_weaponComp->m_weaponsData[i].m_ammunationsCount;
        m_memPlayerConf.m_weapons[i] = m_weaponComp->m_weaponsData[i].m_posses;
    }
    m_memPlayerConf.m_currentWeapon = m_weaponComp->m_currentWeapon;
    m_memPlayerConf.m_previousWeapon = m_weaponComp->m_previousWeapon;
    m_memPlayerConf.m_life = m_playerConf->m_life;
    m_playerMem = true;
}

//===================================================================
void MainEngine::loadPlayerGear()
{
    assert(m_playerConf);
    assert(m_memPlayerConf.m_ammunationsCount.size() ==
           m_weaponComp->m_weaponsData.size());
    for(uint32_t i = 0; i < m_memPlayerConf.m_ammunationsCount.size(); ++i)
    {
        m_weaponComp->m_weaponsData[i].m_ammunationsCount =
                m_memPlayerConf.m_ammunationsCount[i];
        m_weaponComp->m_weaponsData[i].m_posses = m_memPlayerConf.m_weapons[i];
    }
    m_weaponComp->m_currentWeapon = m_memPlayerConf.m_currentWeapon;
    m_weaponComp->m_previousWeapon = m_memPlayerConf.m_previousWeapon;
    m_playerConf->m_life = m_memPlayerConf.m_life;
    m_playerMem = false;
    StaticDisplaySystem *staticDisplay = m_ecsManager.getSystemManager().
            searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    assert(staticDisplay);
    //update FPS weapon sprite
    //weapon type weapon sprite
    std::map<uint32_t, uint32_t>::const_iterator it =
            staticDisplay->getWeaponsSpriteAssociated().find(m_weaponComp->m_currentWeapon);
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
void MainEngine::confPlayerVisibleShoot(const std::vector<uint32_t> &playerVisibleShots,
                                        const pairFloat_t &point, float degreeAngle)
{
    m_physicalEngine.confPlayerVisibleShoot(playerVisibleShots, point, degreeAngle);
}

//===================================================================
void MainEngine::playerAttack(uint32_t playerEntity, PlayerConfComponent *playerComp, const pairFloat_t &point,
                              float degreeAngle)
{

    assert(m_weaponComp->m_currentWeapon < m_weaponComp->m_weaponsData.size());
    const WeaponData &currentWeapon = m_weaponComp->m_weaponsData[
            m_weaponComp->m_currentWeapon];
    AttackType_e attackType = currentWeapon.m_attackType;
    if(attackType == AttackType_e::MELEE)
    {
        GeneralCollisionComponent *actionGenColl = m_ecsManager.getComponentManager().
                searchComponentByType<GeneralCollisionComponent>(playerComp->m_hitMeleeEntity,
                Components_e::GENERAL_COLLISION_COMPONENT);
        MapCoordComponent *playerMapComp = m_ecsManager.getComponentManager().
                searchComponentByType<MapCoordComponent>(playerEntity,
                Components_e::MAP_COORD_COMPONENT);
        MoveableComponent *playerMoveComp = m_ecsManager.getComponentManager().
                searchComponentByType<MoveableComponent>(playerEntity,
                Components_e::MOVEABLE_COMPONENT);
        MapCoordComponent *actionMapComp = m_ecsManager.getComponentManager().
                searchComponentByType<MapCoordComponent>(playerComp->m_hitMeleeEntity,
                Components_e::MAP_COORD_COMPONENT);
        assert(playerMoveComp);
        assert(playerMapComp);
        assert(actionGenColl);
        assert(playerMapComp);
        confActionShape(actionMapComp, playerMapComp, playerMoveComp, actionGenColl);
        return;
    }
    else if(attackType == AttackType_e::BULLETS)
    {
        for(uint32_t i = 0; i < currentWeapon.m_simultaneousShots; ++i)
        {
            confPlayerBullet(playerComp, point, degreeAngle, i);
        }
    }
    else if(attackType == AttackType_e::VISIBLE_SHOTS)
    {
        confPlayerVisibleShoot(playerComp->
                               m_visibleShootEntities[m_weaponComp->m_currentWeapon],
                point, degreeAngle);
    }
    assert(m_weaponComp->m_weaponsData[m_weaponComp->m_currentWeapon].m_ammunationsCount > 0);
    --m_weaponComp->m_weaponsData[m_weaponComp->m_currentWeapon].m_ammunationsCount;
}

//===================================================================
void MainEngine::confPlayerBullet(PlayerConfComponent *playerComp,
                                  const pairFloat_t &point, float degreeAngle,
                                  uint32_t numBullet)
{
    assert(numBullet < SEGMENT_SHOT_NUMBER);
    GeneralCollisionComponent *genColl = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(*playerComp->m_shootEntities[numBullet],
            Components_e::GENERAL_COLLISION_COMPONENT);
    SegmentCollisionComponent *segmentColl = m_ecsManager.getComponentManager().
            searchComponentByType<SegmentCollisionComponent>(*playerComp->m_shootEntities[numBullet],
            Components_e::SEGMENT_COLLISION_COMPONENT);
    assert(genColl);
    assert(segmentColl);
    confBullet(genColl, segmentColl, CollisionTag_e::BULLET_PLAYER_CT, point, degreeAngle);
}

//===================================================================
void confActionShape(MapCoordComponent *mapCompAction, const MapCoordComponent *playerMapComp,
                     const MoveableComponent *playerMoveComp, GeneralCollisionComponent *genCompAction)
{
    mapCompAction->m_absoluteMapPositionPX = playerMapComp->m_absoluteMapPositionPX;
    moveElementFromAngle(LEVEL_HALF_TILE_SIZE_PX, getRadiantAngle(playerMoveComp->m_degreeOrientation),
                         mapCompAction->m_absoluteMapPositionPX);
    genCompAction->m_active = true;
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
    float diff = std::rand() / ((RAND_MAX + 1u) / 9) - 4.0f;
    segmentColl->m_degreeOrientation = degreeAngle + diff;
    if(segmentColl->m_degreeOrientation < EPSILON_FLOAT)
    {
        segmentColl->m_degreeOrientation += 360.0f;
    }
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

uint32_t MainEngine::createTextureEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
void MainEngine::loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                              const GroundCeilingData &ceilingData,
                                              const LevelManager &levelManager)
{
    uint32_t ceiling, ground;
    m_graphicEngine.clearSystems();
    ground = createBackgroundEntity(&groundData);
    confGroundComponents(ground, groundData, levelManager.getPictureSpriteData());
    ceiling = createBackgroundEntity(&ceilingData);
    confCeilingComponents(ceiling, ceilingData, levelManager.getPictureSpriteData());
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
    else
    {
        bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
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
                                 levelManager.getPictureData().getCeilingData(), 
                                 levelManager);
    loadDamageEntity();
    loadTransitionEntity();
    loadStaticElementEntities(levelManager);
    loadPlayerEntity(levelManager, loadWeaponsEntity(levelManager));
    Level::initLevelElementArray();
    loadWallEntities(levelManager);
    loadDoorEntities(levelManager);
    loadEnemiesEntities(levelManager);
}

//===================================================================
uint32_t MainEngine::loadWeaponsEntity(const LevelManager &levelManager)
{
    uint32_t weaponEntity = createWeaponEntity();
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    const std::vector<WeaponINIData> &vectWeapons = levelManager.getWeaponsData();
    MemSpriteDataComponent *memSprite = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(weaponEntity,
                                                          Components_e::MEM_SPRITE_DATA_COMPONENT);
    MemPositionsVertexComponents *memPosVertex = m_ecsManager.getComponentManager().
            searchComponentByType<MemPositionsVertexComponents>(weaponEntity,
                                                                Components_e::MEM_POSITIONS_VERTEX_COMPONENT);
    WeaponComponent *weaponComp = m_ecsManager.getComponentManager().
            searchComponentByType<WeaponComponent>(weaponEntity,
                                                   Components_e::WEAPON_COMPONENT);
    assert(weaponComp);
    assert(memSprite);
    assert(memPosVertex);
    weaponComp->m_weaponsData.resize(vectWeapons.size());
    for(uint32_t i = 0; i < weaponComp->m_weaponsData.size(); ++i)
    {
        weaponComp->m_weaponsData[i].m_ammunationsCount = 0;
        weaponComp->m_weaponsData[i].m_posses = false;
    }
    weaponComp->m_weaponsData[0].m_ammunationsCount = 1;
    weaponComp->m_weaponsData[1].m_ammunationsCount = 20;
    weaponComp->m_weaponsData[0].m_posses = true;
    weaponComp->m_weaponsData[1].m_posses = true;
    weaponComp->m_currentWeapon = 1;
    weaponComp->m_previousWeapon = 1;
    uint32_t totalSize = 0;
    for(uint32_t i = 0; i < vectWeapons.size(); ++i)
    {
        totalSize += vectWeapons[i].m_spritesData.size();
    }
    memSprite->m_vectSpriteData.reserve(totalSize);
    float posUp, posDown = -1.0f, posLeft, posRight, diffLateral;
    memSprite->m_vectSpriteData.reserve(vectWeapons.size());
    for(uint32_t i = 0; i < vectWeapons.size(); ++i)
    {
        weaponComp->m_weaponsData[i].m_maxAmmunations = vectWeapons[i].m_maxAmmo;
        weaponComp->m_weaponsData[i].m_memPosSprite =
        {memSprite->m_vectSpriteData.size(),
         memSprite->m_vectSpriteData.size() + vectWeapons[i].m_spritesData.size() - 1};
        for(uint32_t j = 0; j < vectWeapons[i].m_spritesData.size(); ++j)
        {
            memSprite->m_vectSpriteData.emplace_back(
                        &vectSprite[vectWeapons[i].m_spritesData[j].m_numSprite]);
            posUp = -1.0f + vectWeapons[i].m_spritesData[j].m_GLSize.second;
            diffLateral = vectWeapons[i].m_spritesData[j].m_GLSize.first / 2.0f;
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
    }
    return weaponEntity;
}

//===================================================================
void MainEngine::loadWallEntities(const LevelManager &levelManager)
{
    const std::map<std::string, WallData> &wallData = levelManager.getWallData();
    MemSpriteDataComponent *memSpriteComp;
    SpriteTextureComponent *spriteComp;
    TimerComponent *timerComp;
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    assert(!Level::getLevelCaseType().empty());
    std::set<pairUI_t> memWall;
    std::pair<std::set<pairUI_t>::const_iterator, bool> itt;
    std::map<std::string, WallData>::const_iterator iter = wallData.begin();
    bool multiSprite;
    for(; iter != wallData.end(); ++iter)
    {
        assert(!iter->second.m_sprites.empty());
        assert(iter->second.m_sprites[0] < vectSprite.size());
        const SpriteData &memSpriteData = vectSprite[iter->second.m_sprites[0]];
        multiSprite = (iter->second.m_sprites.size() > 1);
        for(std::set<pairUI_t>::const_iterator it = iter->second.m_TileGamePosition.begin();
            it != iter->second.m_TileGamePosition.end(); ++it)
        {
            itt = memWall.insert(*it);
            if(!itt.second)
            {
                continue;
            }
            uint32_t numEntity = createWallEntity(multiSprite);
            confBaseComponent(numEntity, memSpriteData, (*it),
                              CollisionShape_e::RECTANGLE_C, CollisionTag_e::WALL_CT);
            spriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
            assert(spriteComp);
            Level::addElementCase(spriteComp, (*it), LevelCaseType_e::WALL_LC, numEntity);
            if(!multiSprite)
            {
                continue;
            }
            memSpriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MemSpriteDataComponent>(numEntity,
                                                                  Components_e::MEM_SPRITE_DATA_COMPONENT);
            assert(memSpriteComp);
            uint32_t vectSize = iter->second.m_sprites.size();
            memSpriteComp->m_vectSpriteData.reserve(static_cast<uint32_t>(WallSpriteType_e::TOTAL_SPRITE));
            for(uint32_t j = 0; j < vectSize; ++j)
            {
                memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[iter->second.m_sprites[j]]);
            }
            timerComp = m_ecsManager.getComponentManager().
                    searchComponentByType<TimerComponent>(numEntity, Components_e::TIMER_COMPONENT);
            assert(timerComp);
            timerComp->m_clockA = std::chrono::system_clock::now();
        }
    }
}

//===================================================================
void MainEngine::loadDoorEntities(const LevelManager &levelManager)
{
    const std::map<std::string, DoorData> &doorData = levelManager.getDoorData();
    MemSpriteDataComponent *memSpriteComp;
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    std::map<std::string, DoorData>::const_iterator it = doorData.begin();
    for(; it != doorData.end(); ++it)
    {
        const SpriteData &memSpriteData = levelManager.getPictureData().getSpriteData()[it->second.m_numSprite];
        for(uint32_t j = 0; j < it->second.m_TileGamePosition.size(); ++j)
        {
            uint32_t numEntity = createDoorEntity();
            confBaseComponent(numEntity, memSpriteData, it->second.m_TileGamePosition[j],
                              CollisionShape_e::RECTANGLE_C, CollisionTag_e::DOOR_CT);
            MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MapCoordComponent>(numEntity, Components_e::MAP_COORD_COMPONENT);
            assert(mapComp);
            RectangleCollisionComponent *rectComp = m_ecsManager.getComponentManager().
                    searchComponentByType<RectangleCollisionComponent>(numEntity, Components_e::RECTANGLE_COLLISION_COMPONENT);
            assert(rectComp);
            DoorComponent *doorComp = m_ecsManager.getComponentManager().
                    searchComponentByType<DoorComponent>(numEntity, Components_e::DOOR_COMPONENT);
            assert(doorComp);
            if(it->second.m_vertical)
            {
                mapComp->m_absoluteMapPositionPX.first += DOOR_CASE_POS_PX;
                rectComp->m_size = {WIDTH_DOOR_SIZE_PX, LEVEL_TILE_SIZE_PX};
            }
            else
            {
                mapComp->m_absoluteMapPositionPX.second += DOOR_CASE_POS_PX;
                rectComp->m_size = {LEVEL_TILE_SIZE_PX, WIDTH_DOOR_SIZE_PX};
            }
            doorComp->m_vertical = it->second.m_vertical;
            memSpriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MemSpriteDataComponent>(numEntity,
                                                                  Components_e::MEM_SPRITE_DATA_COMPONENT);

            SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
            assert(spriteComp);
            Level::addElementCase(spriteComp, it->second.m_TileGamePosition[j], LevelCaseType_e::DOOR_LC, numEntity);

            if(!memSpriteComp)
            {
                continue;
            }
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[it->second.m_numSprite]);
        }
    }
}

//===================================================================
void MainEngine::loadEnemiesEntities(const LevelManager &levelManager)
{
    const std::map<std::string, EnemyData> &enemiesData = levelManager.getEnemiesData();
    std::map<std::string, EnemyData>::const_iterator it = enemiesData.begin();
    float collisionRay;
    for(; it != enemiesData.end(); ++it)
    {
        collisionRay = it->second.m_inGameSpriteSize.first * LEVEL_HALF_TILE_SIZE_PX;
        const SpriteData &memSpriteData = levelManager.getPictureData().
                getSpriteData()[it->second.m_staticFrontSprites[0]];
        for(uint32_t j = 0; j < it->second.m_TileGamePosition.size(); ++j)
        {
            uint32_t numEntity = createEnemyEntity();
            confBaseComponent(numEntity, memSpriteData, it->second.m_TileGamePosition[j],
                              CollisionShape_e::CIRCLE_C, CollisionTag_e::ENEMY_CT);
            EnemyConfComponent *enemyComp = m_ecsManager.getComponentManager().
                    searchComponentByType<EnemyConfComponent>(numEntity,
                                                              Components_e::ENEMY_CONF_COMPONENT);
            FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
                    searchComponentByType<FPSVisibleStaticElementComponent>(
                        numEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
            assert(enemyComp);
            assert(fpsStaticComp);
            fpsStaticComp->m_inGameSpriteSize = it->second.m_inGameSpriteSize;
            CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
                    searchComponentByType<CircleCollisionComponent>(numEntity, Components_e::CIRCLE_COLLISION_COMPONENT);
            assert(circleComp);
            circleComp->m_ray = collisionRay;
            enemyComp->m_visibleShot = !(it->second.m_visibleShotSprites.empty());
            if(enemyComp->m_visibleShot)
            {
                createAmmoEntities(enemyComp->m_visibleAmmo, CollisionTag_e::BULLET_ENEMY_CT, enemyComp->m_visibleShot);
            }
            else
            {
                createAmmoEntities(enemyComp->m_stdAmmo, CollisionTag_e::BULLET_ENEMY_CT, enemyComp->m_visibleShot);
            }
            loadEnemySprites(levelManager.getPictureData().getSpriteData(),
                             it->second, numEntity, enemyComp);
            MoveableComponent *moveComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MoveableComponent>(numEntity,
                                                              Components_e::MOVEABLE_COMPONENT);
            assert(moveComp);
            moveComp->m_velocity = 1.0f;
        }
    }
}

//===================================================================
void MainEngine::createAmmoEntities(AmmoContainer_t &ammoCount,
                                     CollisionTag_e collTag,
                                     bool visibleShot)
{
    for(uint32_t i = 0; i < ammoCount.size(); ++i)
    {
        createAmmoEntity(*ammoCount[i], collTag, visibleShot);
    }
}

//===================================================================
void MainEngine::createAmmoEntity(uint32_t &ammoNum, CollisionTag_e collTag,
                                  bool visibleShot)
{
    if(!visibleShot)
    {
        ammoNum = createShotEntity();
    }
    else
    {
        ammoNum = createVisibleShotEntity();
    }
    GeneralCollisionComponent *genColl = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(ammoNum,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genColl);
    genColl->m_active = false;
    genColl->m_tag = collTag;
    genColl->m_shape = (visibleShot) ? CollisionShape_e::CIRCLE_C : CollisionShape_e::SEGMENT_C;
    if(visibleShot)
    {
        confVisibleAmmo(ammoNum);
    }
}

//===================================================================
void MainEngine::createPlayerVisibleShotEntity(mapUiVectUI_t &visibleShots,
                                               WeaponComponent *weaponConf)
{
    for(uint32_t i = 0; weaponConf->m_weaponsData.size(); ++i)
    {
        if(weaponConf->m_weaponsData[i].m_attackType == AttackType_e::VISIBLE_SHOTS)
        {
            visibleShots.insert({i, std::vector<uint32_t>()});
            visibleShots[i].resize(4);
            for(uint32_t j = 0; j < 4; ++j)
            {
                createAmmoEntity(visibleShots[i][j], CollisionTag_e::BULLET_PLAYER_CT,
                                 true);
            }
        }
    }
}

//===================================================================
void MainEngine::createShotImpactEntities(const std::vector<SpriteData> &vectSpriteData,
                                          const std::vector<ShootDisplayData> &shootDisplayData,
                                          const AmmoContainer_t &ammoContainer)
{
    for(uint32_t i = 0; i < shootDisplayData.size(); ++i)
    {
        if(shootDisplayData[i].m_impact.empty())
        {
            continue;
        }
        for(uint32_t j = 0; j < ammoContainer.size(); ++j)
        {
            ShotConfComponent *shotComp = m_ecsManager.getComponentManager().
                    searchComponentByType<ShotConfComponent>(*ammoContainer[j],
                                                             Components_e::SHOT_CONF_COMPONENT);
            assert(shotComp);
            shotComp->m_impactEntity = createShotImpactEntity();
            FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
                    searchComponentByType<FPSVisibleStaticElementComponent>(
                        shotComp->m_impactEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
            GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(shotComp->m_impactEntity,
                                                                     Components_e::GENERAL_COLLISION_COMPONENT);
            assert(genComp);
            assert(fpsStaticComp);
            fpsStaticComp->m_inGameSpriteSize = shootDisplayData[i].m_impact[0].m_GLSize;
            fpsStaticComp->m_levelElementType = LevelStaticElementType_e::IMPACT;
            genComp->m_active = false;
            genComp->m_tag = CollisionTag_e::IMPACT_CT;
            genComp->m_shape = CollisionShape_e::CIRCLE_C;
            loadShotImpactSprite(vectSpriteData, shootDisplayData[i], shotComp->m_impactEntity);
        }
    }
}

//===================================================================
void MainEngine::loadEnemySprites(const std::vector<SpriteData> &vectSprite,
                                  const EnemyData &enemiesData, uint32_t numEntity,
                                  EnemyConfComponent *enemyComp)
{
    MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(numEntity,
                                                          Components_e::MEM_SPRITE_DATA_COMPONENT);
    assert(memSpriteComp);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticFrontSprites,
                              EnemySpriteType_e::STATIC_FRONT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticFrontLeftSprites,
                              EnemySpriteType_e::STATIC_FRONT_LEFT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticFrontRightSprites,
                              EnemySpriteType_e::STATIC_FRONT_RIGHT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticBackSprites,
                              EnemySpriteType_e::STATIC_BACK);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticBackLeftSprites,
                              EnemySpriteType_e::STATIC_BACK_LEFT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticBackRightSprites,
                              EnemySpriteType_e::STATIC_BACK_RIGHT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticLeftSprites,
                              EnemySpriteType_e::STATIC_LEFT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticRightSprites,
                              EnemySpriteType_e::STATIC_RIGHT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_attackSprites,
                              EnemySpriteType_e::ATTACK);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_dyingSprites,
                              EnemySpriteType_e::DYING);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate,
                              memSpriteComp->m_vectSpriteData,
                              enemiesData.m_touched,
                              EnemySpriteType_e::TOUCHED);
    if(enemyComp->m_visibleShot)
    {
        loadVisibleShotEnemySprites(vectSprite, enemyComp->m_visibleAmmo, enemiesData);
    }
}

//===================================================================
void insertEnemySpriteFromType(const std::vector<SpriteData> &vectSprite,
                               mapEnemySprite_t &mapSpriteAssociate,
                               std::vector<SpriteData const *> &vectSpriteData,
                               const std::vector<uint8_t> &enemyMemArray,
                               EnemySpriteType_e type)
{
    //second pair {first pos last pos}
    mapSpriteAssociate.insert({type, {vectSpriteData.size(), vectSpriteData.size() +
                                      enemyMemArray.size() - 1}});
    for(uint32_t j = 0; j < enemyMemArray.size(); ++j)
    {
        vectSpriteData.emplace_back(&vectSprite[enemyMemArray[j]]);
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
                m_visibleShotSprites[shotConfComp->m_spriteShotNum]];
    }
}

//===================================================================
void MainEngine::confVisibleAmmo(uint32_t ammoEntity)
{
    pairFloat_t pairSpriteSize = {0.2f, 0.3f};
    float collisionRay = pairSpriteSize.first * LEVEL_HALF_TILE_SIZE_PX;
    CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
            searchComponentByType<CircleCollisionComponent>(ammoEntity,
                                                            Components_e::CIRCLE_COLLISION_COMPONENT);
    FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
            searchComponentByType<FPSVisibleStaticElementComponent>(
                ammoEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    MoveableComponent *moveComp = m_ecsManager.getComponentManager().
            searchComponentByType<MoveableComponent>(
                ammoEntity, Components_e::MOVEABLE_COMPONENT);
    assert(circleComp);
    assert(fpsStaticComp);
    assert(moveComp);
    circleComp->m_ray = collisionRay;
    fpsStaticComp->m_inGameSpriteSize = pairSpriteSize;
    moveComp->m_velocity = 5.0f;
}

//===================================================================
uint32_t MainEngine::createWeaponEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_POSITIONS_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    bitsetComponents[Components_e::WEAPON_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createWallEntity(bool multiSprite)
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::RECTANGLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    if(multiSprite)
    {
        bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
        bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    }
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
uint32_t MainEngine::createShotImpactEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::IMPACT_CONF_COMPONENT] = true;
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
    if(tag == CollisionTag_e::EXIT_CT || tag == CollisionTag_e::OBJECT_CT ||
            tag == CollisionTag_e::ENEMY_CT || tag == CollisionTag_e::STATIC_SET_CT)
    {
        mapComp->m_absoluteMapPositionPX = getCenteredAbsolutePosition(coordLevel);
    }
    else
    {
        mapComp->m_absoluteMapPositionPX = getAbsolutePosition(coordLevel);
    }
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
void MainEngine::loadPlayerEntity(const LevelManager &levelManager,
                                  uint32_t numWeaponEntity)
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
    confPlayerEntity(levelManager, entityNum, levelManager.getLevel(), numWeaponEntity);
    //notify player entity number
    m_graphicEngine.getMapSystem().confPlayerComp(entityNum);
    m_physicalEngine.memPlayerEntity(entityNum);
}

//===================================================================
void MainEngine::confPlayerEntity(const LevelManager &levelManager,
                                  uint32_t entityNum, const Level &level,
                                  uint32_t numWeaponEntity)
{
    const std::vector<SpriteData> &vectSpriteData =
            levelManager.getPictureData().getSpriteData();
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
    playerConf->m_weaponEntity = numWeaponEntity;
    WeaponComponent *weaponConf = m_ecsManager.getComponentManager().
            searchComponentByType<WeaponComponent>(playerConf->m_weaponEntity,
                                                   Components_e::WEAPON_COMPONENT);
    assert(pos);
    assert(map);
    assert(move);
    assert(color);
    assert(circleColl);
    assert(tagColl);
    assert(vision);
    assert(playerConf);
    assert(weaponConf);
    m_playerConf = playerConf;
    m_weaponComp = weaponConf;
    createAmmoEntities(m_playerConf->m_shootEntities, CollisionTag_e::BULLET_PLAYER_CT, false);
    confShotsEntities(m_playerConf->m_shootEntities, 1);
    createPlayerVisibleShotEntity(m_playerConf->m_visibleShootEntities, weaponConf);
    loadPlayerVisibleShotsSprite(vectSpriteData, levelManager.getShootDisplayData(),
                                 m_playerConf->m_visibleShootEntities);
    createShotImpactEntities(vectSpriteData, levelManager.getShootDisplayData(),
                             m_playerConf->m_shootEntities);

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
    map->m_absoluteMapPositionPX = getCenteredAbsolutePosition(map->m_coord);
    updatePlayerOrientation(*move, *pos, *vision);
    color->m_vertex.reserve(3);
    color->m_vertex.emplace_back(tupleTetraFloat{0.9f, 0.00f, 0.00f, 1.0f});
    color->m_vertex.emplace_back(tupleTetraFloat{0.9f, 0.00f, 0.00f, 1.0f});
    color->m_vertex.emplace_back(tupleTetraFloat{0.9f, 0.00f, 0.00f, 1.0f});
    circleColl->m_ray = PLAYER_RAY;
    tagColl->m_tag = CollisionTag_e::PLAYER_CT;
    tagColl->m_shape = CollisionShape_e::CIRCLE_C;
    //set standard weapon sprite
    StaticDisplaySystem *staticDisplay = m_ecsManager.getSystemManager().
            searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    assert(staticDisplay);
    staticDisplay->setWeaponSprite(numWeaponEntity, 0);
    confWriteEntities();
    confMenuCursorEntity();
    confActionEntity();
    confAxeHitEntity();
}

//===================================================================
void MainEngine::confActionEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    uint32_t entityNum = m_ecsManager.addEntity(bitsetComponents);
    GeneralCollisionComponent *genCollComp = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(entityNum,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
            searchComponentByType<MapCoordComponent>(entityNum,
                                                     Components_e::MAP_COORD_COMPONENT);
    CircleCollisionComponent *circleColl = m_ecsManager.getComponentManager().
            searchComponentByType<CircleCollisionComponent>(entityNum,
                                                     Components_e::CIRCLE_COLLISION_COMPONENT);
    assert(genCollComp);
    assert(mapComp);
    assert(circleColl);
    genCollComp->m_active = false;
    genCollComp->m_shape = CollisionShape_e::CIRCLE_C;
    genCollComp->m_tag = CollisionTag_e::PLAYER_ACTION_CT;
    circleColl->m_ray = 15.0f;
    m_playerConf->m_actionEntity = entityNum;
}

//===================================================================
void MainEngine::confAxeHitEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::SHOT_CONF_COMPONENT] = true;
    uint32_t entityNum = m_ecsManager.addEntity(bitsetComponents);
    GeneralCollisionComponent *genCollComp = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(entityNum,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
            searchComponentByType<MapCoordComponent>(entityNum,
                                                     Components_e::MAP_COORD_COMPONENT);
    CircleCollisionComponent *circleColl = m_ecsManager.getComponentManager().
            searchComponentByType<CircleCollisionComponent>(entityNum,
                                                     Components_e::CIRCLE_COLLISION_COMPONENT);
    ShotConfComponent *shotComp = m_ecsManager.getComponentManager().
            searchComponentByType<ShotConfComponent>(entityNum,
                                                     Components_e::SHOT_CONF_COMPONENT);
    assert(shotComp);
    assert(genCollComp);
    assert(mapComp);
    assert(circleColl);
    genCollComp->m_active = false;
    genCollComp->m_shape = CollisionShape_e::CIRCLE_C;
    genCollComp->m_tag = CollisionTag_e::HIT_PLAYER_CT;
    circleColl->m_ray = 10.0f;
    shotComp->m_damage = 1;
    m_playerConf->m_hitMeleeEntity = entityNum;
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
void MainEngine::loadShotImpactSprite(const std::vector<SpriteData> &vectSpriteData,
                                      const ShootDisplayData &shootDisplayData,
                                      uint32_t impactEntity)
{
    MemSpriteDataComponent *memComp = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(impactEntity,
                                                          Components_e::MEM_SPRITE_DATA_COMPONENT);
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(impactEntity,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(memComp);
    assert(spriteComp);
    for(uint32_t l = 0; l < shootDisplayData.m_impact.size(); ++l)
    {
        memComp->m_vectSpriteData.emplace_back(
                    &vectSpriteData[shootDisplayData.m_impact[l].m_numSprite]);
    }
    spriteComp->m_spriteData = memComp->m_vectSpriteData[0];
}

//===================================================================
void MainEngine::loadPlayerVisibleShotsSprite(const std::vector<SpriteData> &vectSpriteData,
                                              const std::vector<ShootDisplayData> &shootDisplayData,
                                              const mapUiVectUI_t &ammoEntities)
{
    mapUiVectUI_t::const_iterator it = ammoEntities.begin();
    for(; it != ammoEntities.end(); ++it)
    {
        for(uint32_t i = 0; i < (*it).second.size(); ++i)
        {
            MemSpriteDataComponent *memComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MemSpriteDataComponent>((*it).second[i],
                                                                  Components_e::MEM_SPRITE_DATA_COMPONENT);
            SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<SpriteTextureComponent>((*it).second[i],
                                                                  Components_e::SPRITE_TEXTURE_COMPONENT);
            assert(memComp);
            assert(spriteComp);
            for(uint32_t l = 0; l < shootDisplayData[i].m_active.size(); ++l)
            {
                memComp->m_vectSpriteData.emplace_back(&vectSpriteData[shootDisplayData[i].m_active[l].m_numSprite]);
            }
            for(uint32_t l = 0; l < shootDisplayData[i].m_destruct.size(); ++l)
            {
                memComp->m_vectSpriteData.emplace_back(&vectSpriteData[shootDisplayData[i].m_destruct[l].m_numSprite]);
            }
            spriteComp->m_spriteData = memComp->m_vectSpriteData[0];
        }
    }
}

//===================================================================
void MainEngine::confWriteEntities()
{
    uint32_t numAmmoWrite = createWriteEntity(),
            numLifeWrite = createWriteEntity(), numMenuWrite = createWriteEntity();
    //AMMO
    WriteComponent *writeConf = m_ecsManager.getComponentManager().
            searchComponentByType<WriteComponent>(numAmmoWrite, Components_e::WRITE_COMPONENT);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.95f, -0.9f};
    writeConf->m_fontSize = STD_FONT_SIZE;
    m_graphicEngine.updateAmmoCount(writeConf, m_weaponComp);
    //LIFE
    writeConf = m_ecsManager.getComponentManager().
            searchComponentByType<WriteComponent>(numLifeWrite, Components_e::WRITE_COMPONENT);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.95f, -0.8f};
    writeConf->m_fontSize = STD_FONT_SIZE;
    m_graphicEngine.updatePlayerLife(writeConf, m_playerConf);
    //MENU
    writeConf = m_ecsManager.getComponentManager().
            searchComponentByType<WriteComponent>(numMenuWrite, Components_e::WRITE_COMPONENT);
    assert(writeConf);
    m_writeConf = writeConf;
    writeConf->m_upLeftPositionGL = m_menuCornerUpLeft;
    writeConf->m_fontSize = MENU_FONT_SIZE;
    m_graphicEngine.fillMenuWrite(writeConf, MENU_ENTRIES);
    m_playerConf->m_menuEntity = numMenuWrite;
    m_playerConf->m_ammoWriteEntity = numAmmoWrite;
    m_playerConf->m_lifeWriteEntity = numLifeWrite;
}

//===================================================================
void MainEngine::confMenuCursorEntity()
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
    m_playerConf->m_menuCursorEntity = cursorEntity;
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
    loadStaticElementGroup(levelManager, levelManager.getGroundData(),
                           LevelStaticElementType_e::GROUND);
    loadStaticElementGroup(levelManager, levelManager.getCeilingData(),
                           LevelStaticElementType_e::CEILING);
    loadStaticElementGroup(levelManager, levelManager.getObjectData(),
                           LevelStaticElementType_e::OBJECT);
    loadExitElement(levelManager, levelManager.getExitElementData());
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
                                        const std::map<std::string, StaticLevelElementData> &staticData,
                                        LevelStaticElementType_e elementType)
{
    CollisionTag_e tag;
    std::map<std::string, StaticLevelElementData>::const_iterator it = staticData.begin();
    float collisionRay;
    for(; it != staticData.end(); ++it)
    {
        collisionRay = it->second.m_inGameSpriteSize.first * LEVEL_HALF_TILE_SIZE_PX;
        const SpriteData &memSpriteData = levelManager.getPictureData().
                getSpriteData()[it->second.m_numSprite];
        for(uint32_t j = 0; j < it->second.m_TileGamePosition.size(); ++j)
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
                objComp->m_containing = it->second.m_containing;
                objComp->m_type = it->second.m_type;
            }
            else
            {
                if(it->second.m_traversable)
                {
                    tag = CollisionTag_e::GHOST_CT;
                }
                else
                {
                    tag = CollisionTag_e::STATIC_SET_CT;
                }
                entityNum = createStaticEntity();
            }
            FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
                    searchComponentByType<FPSVisibleStaticElementComponent>(
                        entityNum, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
            assert(fpsStaticComp);
            fpsStaticComp->m_inGameSpriteSize = it->second.m_inGameSpriteSize;
            confBaseComponent(entityNum, memSpriteData, it->second.m_TileGamePosition[j],
                    CollisionShape_e::CIRCLE_C, tag);
            CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
                    searchComponentByType<CircleCollisionComponent>(entityNum, Components_e::CIRCLE_COLLISION_COMPONENT);
            assert(circleComp);
            circleComp->m_ray = collisionRay;
            confStaticComponent(entityNum, it->second.m_inGameSpriteSize, elementType);
        }
    }
}

//===================================================================
void MainEngine::confGroundComponents(uint32_t entityNum,
                                      const GroundCeilingData &groundData,
                                      const std::vector<SpriteData> &vectSprite)
{
    if(groundData.m_apparence == DisplayType_e::COLOR)
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
        colorComp->m_vertex.emplace_back(std::get<0>(groundData.m_color[0]),
                std::get<1>(groundData.m_color[0]), std::get<2>(groundData.m_color[0]),
                1.0);
        colorComp->m_vertex.emplace_back(std::get<0>(groundData.m_color[1]),
                std::get<1>(groundData.m_color[1]), std::get<2>(groundData.m_color[1]),
                1.0);
        colorComp->m_vertex.emplace_back(std::get<0>(groundData.m_color[2]),
                std::get<1>(groundData.m_color[2]), std::get<2>(groundData.m_color[2]),
                1.0);
        colorComp->m_vertex.emplace_back(std::get<0>(groundData.m_color[3]),
                std::get<1>(groundData.m_color[3]), std::get<2>(groundData.m_color[3]),
                1.0);
        memColorSystemEntity(entityNum);
    }
    else
    {
        bool simpleTexture = (groundData.m_apparence == DisplayType_e::SIMPLE_TEXTURE);
        if(simpleTexture)
        {
            PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
                    searchComponentByType<PositionVertexComponent>(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
            assert(posComp);
            posComp->m_vertex.reserve(4);
            posComp->m_vertex.emplace_back(-1.0f, 0.0f);
            posComp->m_vertex.emplace_back(1.0f, 0.0f);
            posComp->m_vertex.emplace_back(1.0f, -1.0f);
            posComp->m_vertex.emplace_back(-1.0f, -1.0f);
        }
        SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                searchComponentByType<SpriteTextureComponent>(entityNum, Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        assert(vectSprite.size() >= groundData.m_spriteNum);
        spriteComp->m_spriteData = &vectSprite[groundData.m_spriteNum];
        memGroundBackgroundFPSSystemEntity(entityNum, simpleTexture);
    }
}

//===================================================================
void MainEngine::confCeilingComponents(uint32_t entityNum, const
                                       GroundCeilingData &ceilingData, const std::vector<SpriteData> &vectSprite)
{

    if(ceilingData.m_apparence == DisplayType_e::COLOR)
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
        colorComp->m_vertex.emplace_back(std::get<0>(ceilingData.m_color[0]),
                std::get<1>(ceilingData.m_color[0]), std::get<2>(ceilingData.m_color[0]),
                1.0);
        colorComp->m_vertex.emplace_back(std::get<0>(ceilingData.m_color[1]),
                std::get<1>(ceilingData.m_color[1]), std::get<2>(ceilingData.m_color[1]),
                1.0);
        colorComp->m_vertex.emplace_back(std::get<0>(ceilingData.m_color[2]),
                std::get<1>(ceilingData.m_color[2]), std::get<2>(ceilingData.m_color[2]),
                1.0);
        colorComp->m_vertex.emplace_back(std::get<0>(ceilingData.m_color[3]),
                std::get<1>(ceilingData.m_color[3]), std::get<2>(ceilingData.m_color[3]),
                1.0);
        memColorSystemEntity(entityNum);
    }
    else
    {
        bool simpleTexture = ceilingData.m_apparence == DisplayType_e::SIMPLE_TEXTURE;
        if(simpleTexture)
        {
            PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
                    searchComponentByType<PositionVertexComponent>(entityNum, Components_e::POSITION_VERTEX_COMPONENT);
            assert(posComp);
            posComp->m_vertex.reserve(4);
            posComp->m_vertex.emplace_back(-1.0f, 1.0f);
            posComp->m_vertex.emplace_back(1.0f, 1.0f);
            posComp->m_vertex.emplace_back(1.0f, 0.0f);
            posComp->m_vertex.emplace_back(-1.0f, 0.0f);
        }
            SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<SpriteTextureComponent>(entityNum, Components_e::SPRITE_TEXTURE_COMPONENT);
            assert(spriteComp);
            spriteComp->m_spriteData = &vectSprite[ceilingData.m_spriteNum];
            memCeilingBackgroundFPSSystemEntity(entityNum, simpleTexture);
    }
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
