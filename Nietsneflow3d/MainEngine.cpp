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
#include <ECS/Components/MemFPSGLSizeComponent.hpp>
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/TeleportComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/MemPositionsVertexComponents.hpp>
#include <ECS/Components/SegmentCollisionComponent.hpp>
#include <ECS/Components/TriggerComponent.hpp>
#include <ECS/Components/WriteComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/ShotConfComponent.hpp>
#include <ECS/Components/WeaponComponent.hpp>
#include <ECS/Components/MoveableWallConfComponent.hpp>
#include <ECS/Components/BarrelComponent.hpp>
#include <ECS/Systems/ColorDisplaySystem.hpp>
#include <ECS/Systems/MapDisplaySystem.hpp>
#include <ECS/Systems/CollisionSystem.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <ECS/Systems/VisionSystem.hpp>
#include <ECS/Systems/DoorWallSystem.hpp>
#include <ECS/Systems/StaticDisplaySystem.hpp>
#include <ECS/Systems/IASystem.hpp>
#include <LevelManager.hpp>
#include <cassert>
#include <bitset>

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
    m_memPlayerConf.m_weapons.resize(m_weaponComp->m_weaponsData.size());
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
    staticDisplay->setWeaponSprite(m_playerConf->m_weaponEntity, m_weaponComp->m_weaponsData[m_weaponComp->m_currentWeapon].m_memPosSprite.first);
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
void MainEngine::confPlayerVisibleShoot(std::vector<uint32_t> &playerVisibleShots,
                                        const PairFloat_t &point, float degreeAngle)
{
    m_physicalEngine.confPlayerVisibleShoot(playerVisibleShots, point, degreeAngle);
}

//===================================================================
void MainEngine::playerAttack(uint32_t playerEntity, PlayerConfComponent *playerComp, const PairFloat_t &point,
                              float degreeAngle)
{

    assert(m_weaponComp->m_currentWeapon < m_weaponComp->m_weaponsData.size());
    WeaponData &currentWeapon = m_weaponComp->m_weaponsData[
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
        confActionShape(actionMapComp, actionGenColl, playerMapComp, playerMoveComp);
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
        assert(currentWeapon.m_visibleShootEntities);
        confPlayerVisibleShoot((*currentWeapon.m_visibleShootEntities), point, degreeAngle);
    }
    assert(m_weaponComp->m_weaponsData[m_weaponComp->m_currentWeapon].m_ammunationsCount > 0);
    --m_weaponComp->m_weaponsData[m_weaponComp->m_currentWeapon].m_ammunationsCount;
}

//===================================================================
void MainEngine::confPlayerBullet(PlayerConfComponent *playerComp,
                                  const PairFloat_t &point, float degreeAngle,
                                  uint32_t numBullet)
{
    assert(numBullet < MAX_SHOTS);
    WeaponComponent *weaponComp = m_ecsManager.getComponentManager().
            searchComponentByType<WeaponComponent>(playerComp->m_weaponEntity,
            Components_e::WEAPON_COMPONENT);
    assert(weaponComp);
    uint32_t bulletEntity = (*weaponComp->m_weaponsData[weaponComp->m_currentWeapon].
            m_segmentShootEntities)[numBullet];
    GeneralCollisionComponent *genColl = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(bulletEntity,
            Components_e::GENERAL_COLLISION_COMPONENT);
    SegmentCollisionComponent *segmentColl = m_ecsManager.getComponentManager().
            searchComponentByType<SegmentCollisionComponent>(bulletEntity,
            Components_e::SEGMENT_COLLISION_COMPONENT);
    assert(genColl);
    assert(segmentColl);
    confBullet(genColl, segmentColl, CollisionTag_e::BULLET_PLAYER_CT, point, degreeAngle);
}

//===================================================================
void confActionShape(MapCoordComponent *mapCompAction, GeneralCollisionComponent *genCompAction,
                     const MapCoordComponent *attackerMapComp, const MoveableComponent *attackerMoveComp)
{
    mapCompAction->m_absoluteMapPositionPX = attackerMapComp->m_absoluteMapPositionPX;
    moveElementFromAngle(LEVEL_HALF_TILE_SIZE_PX, getRadiantAngle(attackerMoveComp->m_degreeOrientation),
                         mapCompAction->m_absoluteMapPositionPX);
    genCompAction->m_active = true;
}

//===================================================================
void confBullet(GeneralCollisionComponent *genColl, SegmentCollisionComponent *segmentColl,
                CollisionTag_e collTag, const PairFloat_t &point, float degreeAngle)
{
    assert(collTag == CollisionTag_e::BULLET_ENEMY_CT ||
           collTag == CollisionTag_e::BULLET_PLAYER_CT);
    genColl->m_tagA = collTag;
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
    std::vector<uint32_t> vectEntities = m_ecsManager.getEntitiesContainingComponents(bitset);
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
void MainEngine::loadColorEntities()
{
    uint32_t damageEntity = createColorEntity(),
            getObjectEntity = createColorEntity(),
            transitionEntity = createColorEntity();
    confUnifiedColorEntity(transitionEntity, {0.0f, 0.0f, 0.0f});
    confUnifiedColorEntity(damageEntity, {0.7f, 0.2f, 0.1f});
    confUnifiedColorEntity(getObjectEntity, {0.1f, 0.7f, 0.5f});
    m_ecsManager.getSystemManager().searchSystemByType<ColorDisplaySystem>(
                static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->
            loadColorEntities(damageEntity, getObjectEntity, transitionEntity);
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
                                     std::get<2>(color), 0.4f);
    colorComp->m_vertex.emplace_back(std::get<0>(color), std::get<1>(color),
                                     std::get<2>(color), 0.4f);
    colorComp->m_vertex.emplace_back(std::get<0>(color), std::get<1>(color),
                                     std::get<2>(color), 0.4f);
    colorComp->m_vertex.emplace_back(std::get<0>(color), std::get<1>(color),
                                     std::get<2>(color), 0.4f);
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
void MainEngine::loadBackgroundEntities(const GroundCeilingData &groundData, const GroundCeilingData &ceilingData,
                                        const LevelManager &levelManager)
{

    uint32_t entity, colorIndex = static_cast<uint32_t>(DisplayType_e::COLOR),
            simpleTextIndex = static_cast<uint32_t>(DisplayType_e::SIMPLE_TEXTURE),
            tiledTextIndex = static_cast<uint32_t>(DisplayType_e::TEXTURED_TILE);
    if(groundData.m_apparence[simpleTextIndex])
    {
        entity = createBackgroundEntity(false);
        confGroundSimpleTextBackgroundComponents(entity, groundData, levelManager.getPictureSpriteData());
        memGroundBackgroundFPSSystemEntity(entity, true);
    }
    else if(groundData.m_apparence[colorIndex])
    {
        entity = createBackgroundEntity(true);
        confColorBackgroundComponents(entity, groundData, true);
        memColorSystemEntity(entity);
    }
    if(groundData.m_apparence[tiledTextIndex])
    {
        entity = createBackgroundEntity(false);
        confTiledTextBackgroundComponents(entity, groundData, levelManager.getPictureSpriteData());
        memGroundBackgroundFPSSystemEntity(entity, false);
    }

    if(ceilingData.m_apparence[simpleTextIndex])
    {
        entity = createBackgroundEntity(false);
        confCeilingSimpleTextBackgroundComponents(entity, ceilingData, levelManager.getPictureSpriteData());
        memCeilingBackgroundFPSSystemEntity(entity, true);
    }
    else if(ceilingData.m_apparence[colorIndex])
    {
        entity = createBackgroundEntity(true);
        confColorBackgroundComponents(entity, ceilingData, false);
        memColorSystemEntity(entity);
    }
    if(ceilingData.m_apparence[tiledTextIndex])
    {
        entity = createBackgroundEntity(false);
        confTiledTextBackgroundComponents(entity, ceilingData, levelManager.getPictureSpriteData());
        memCeilingBackgroundFPSSystemEntity(entity, false);
    }
}

//===================================================================
uint32_t MainEngine::createBackgroundEntity(bool color)
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    if(color)
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
    m_physicalEngine.clearSystems();
    m_graphicEngine.clearSystems();
    m_memTriggerCreated.clear();
    loadBackgroundEntities(levelManager.getPictureData().getGroundData(),
                           levelManager.getPictureData().getCeilingData(),
                           levelManager);
    loadColorEntities();
    loadStaticElementEntities(levelManager);
    loadBarrelElementEntities(levelManager);
    uint32_t displayTeleportEntity = loadDisplayTeleportEntity(levelManager);
    uint32_t weaponEntity = loadWeaponsEntity(levelManager);
    loadPlayerEntity(levelManager, weaponEntity, displayTeleportEntity);
    Level::initLevelElementArray();
    m_memWall.clear();
    loadWallEntities(levelManager.getWallData(), levelManager.getPictureData().getSpriteData());
    loadMoveableWallEntities(levelManager.getMoveableWallData(), levelManager.getPictureData().getSpriteData());
    loadDoorEntities(levelManager);
    loadEnemiesEntities(levelManager);
}

//===================================================================
uint32_t MainEngine::loadWeaponsEntity(const LevelManager &levelManager)
{
    uint32_t weaponEntity = createWeaponEntity(), weaponToTreat;
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    const std::vector<WeaponINIData> &vectWeapons = levelManager.getWeaponsData();
    MemSpriteDataComponent *memSprite = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(weaponEntity, Components_e::MEM_SPRITE_DATA_COMPONENT);
    MemPositionsVertexComponents *memPosVertex = m_ecsManager.getComponentManager().
            searchComponentByType<MemPositionsVertexComponents>(weaponEntity, Components_e::MEM_POSITIONS_VERTEX_COMPONENT);
    WeaponComponent *weaponComp = m_ecsManager.getComponentManager().
            searchComponentByType<WeaponComponent>(weaponEntity, Components_e::WEAPON_COMPONENT);
    assert(weaponComp);
    assert(memSprite);
    assert(memPosVertex);
    std::vector<bool> checkBitset(vectWeapons.size());
    std::fill(checkBitset.begin(), checkBitset.end(), false);
    weaponComp->m_weaponsData.resize(vectWeapons.size());
    for(uint32_t i = 0; i < weaponComp->m_weaponsData.size(); ++i)
    {
        assert(vectWeapons[i].m_order < checkBitset.size());
        assert(!checkBitset[vectWeapons[i].m_order]);
        checkBitset[vectWeapons[i].m_order] = true;
        weaponComp->m_weaponsData[i].m_ammunationsCount = 0;
        weaponComp->m_weaponsData[i].m_posses = false;
    }
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
        weaponToTreat = vectWeapons[i].m_order;
        if(vectWeapons[i].m_startingPossess)
        {
            weaponComp->m_weaponsData[weaponToTreat].m_posses = true;
            weaponComp->m_currentWeapon = vectWeapons[i].m_order;
            weaponComp->m_previousWeapon = vectWeapons[i].m_order;
        }
        if(vectWeapons[i].m_startingAmmoCount)
        {
            weaponComp->m_weaponsData[weaponToTreat].m_ammunationsCount = *vectWeapons[i].m_startingAmmoCount;
        }
        weaponComp->m_weaponsData[weaponToTreat].m_weaponPower = vectWeapons[i].m_damage;
        weaponComp->m_weaponsData[weaponToTreat].m_animMode = vectWeapons[i].m_animMode;
        weaponComp->m_weaponsData[weaponToTreat].m_latency = vectWeapons[i].m_animationLatency;
        weaponComp->m_weaponsData[weaponToTreat].m_visibleShotID = vectWeapons[i].m_visibleShootID;
        weaponComp->m_weaponsData[weaponToTreat].m_impactID = vectWeapons[i].m_impactID;
        weaponComp->m_weaponsData[weaponToTreat].m_shotVelocity = vectWeapons[i].m_shotVelocity;
        weaponComp->m_weaponsData[weaponToTreat].m_maxAmmunations = vectWeapons[i].m_maxAmmo;
        weaponComp->m_weaponsData[weaponToTreat].m_memPosSprite = {memSprite->m_vectSpriteData.size(),
                                                                   memSprite->m_vectSpriteData.size() + vectWeapons[i].m_spritesData.size() - 1};
        weaponComp->m_weaponsData[weaponToTreat].m_lastAnimNum = memSprite->m_vectSpriteData.size() + vectWeapons[i].m_lastAnimNum;
        weaponComp->m_weaponsData[weaponToTreat].m_attackType = vectWeapons[i].m_attackType;
        weaponComp->m_weaponsData[weaponToTreat].m_simultaneousShots = vectWeapons[i].m_simultaneousShots;

        weaponComp->m_weaponsData[weaponToTreat].m_damageRay = vectWeapons[i].m_damageCircleRay;
        for(uint32_t j = 0; j < vectWeapons[i].m_spritesData.size(); ++j)
        {
            memSprite->m_vectSpriteData.emplace_back(&vectSprite[vectWeapons[i].m_spritesData[j].m_numSprite]);
            posUp = -1.0f + vectWeapons[i].m_spritesData[j].m_GLSize.second;
            diffLateral = vectWeapons[i].m_spritesData[j].m_GLSize.first / 2.0f;
            posLeft = -diffLateral;
            posRight = diffLateral;
            memPosVertex->m_vectSpriteData.emplace_back(std::array<PairFloat_t, 4>{
                                                            {
                                                                {posLeft, posUp},
                                                                {posRight, posUp},
                                                                {posRight, posDown},
                                                                {posLeft, posDown}
                                                            }
                                                        });
        }
    }
    assert(weaponComp->m_currentWeapon < vectWeapons.size());
    return weaponEntity;
}

//===================================================================
void MainEngine::loadWallEntities(const std::map<std::string, WallData> &wallData,
                                  const std::vector<SpriteData> &vectSprite)
{
    assert(!Level::getLevelCaseType().empty());
    std::pair<std::set<PairUI_t>::const_iterator, bool> itt;
    std::map<std::string, WallData>::const_iterator iter = wallData.begin();
    for(; iter != wallData.end(); ++iter)
    {
        assert(!iter->second.m_sprites.empty());
        assert(iter->second.m_sprites[0] < vectSprite.size());
        const SpriteData &memSpriteData = vectSprite[iter->second.m_sprites[0]];
        for(std::set<PairUI_t>::const_iterator it = iter->second.m_TileGamePosition.begin();
            it != iter->second.m_TileGamePosition.end(); ++it)
        {
            itt = m_memWall.insert(*it);
            if(!itt.second)
            {
                continue;
            }
            uint32_t numEntity = createWallEntity(iter->second.m_sprites.size() > 1);
            confBaseWallData(numEntity, memSpriteData, *it, iter->second.m_sprites, vectSprite);
        }
    }
}

//===================================================================
void MainEngine::loadMoveableWallEntities(const std::map<std::string, MoveableWallData> &wallData,
                                          const std::vector<SpriteData> &vectSprite)
{
    assert(!Level::getLevelCaseType().empty());
    std::pair<std::set<PairUI_t>::const_iterator, bool> itt;
    TriggerWallMoveType_e memTriggerType;
    std::vector<uint32_t> vectMemEntities;
    std::map<std::string, MoveableWallData>::const_iterator iter = wallData.begin();
    for(; iter != wallData.end(); ++iter)
    {
        assert(!iter->second.m_sprites.empty());
        assert(iter->second.m_sprites[0] < vectSprite.size());
        const SpriteData &memSpriteData = vectSprite[iter->second.m_sprites[0]];
        memTriggerType = iter->second.m_triggerType;
        vectMemEntities.clear();
        vectMemEntities.reserve(iter->second.m_TileGamePosition.size());
        for(std::set<PairUI_t>::const_iterator it = iter->second.m_TileGamePosition.begin();
            it != iter->second.m_TileGamePosition.end(); ++it)
        {
            itt = m_memWall.insert(*it);
            if(!itt.second)
            {
                continue;
            }
            uint32_t numEntity = createWallEntity(iter->second.m_sprites.size() > 1, true);
            vectMemEntities.emplace_back(numEntity);
            confBaseWallData(numEntity, memSpriteData, *it, iter->second.m_sprites, vectSprite, true);
            MoveableComponent *moveComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MoveableComponent>(numEntity, Components_e::MOVEABLE_COMPONENT);
            assert(moveComp);
            moveComp->m_velocity = iter->second.m_velocity;
            MoveableWallConfComponent *moveWallConfComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MoveableWallConfComponent>(numEntity, Components_e::MOVEABLE_WALL_CONF_COMPONENT);
            assert(moveWallConfComp);
            moveWallConfComp->setIDEntityAssociated(numEntity);
            moveWallConfComp->m_directionMove = iter->second.m_directionMove;
            moveWallConfComp->m_triggerType = iter->second.m_triggerType;
            moveWallConfComp->m_triggerBehaviour = iter->second.m_triggerBehaviourType;
            moveWallConfComp->m_manualTrigger =
                    (moveWallConfComp->m_triggerBehaviour == TriggerBehaviourType_e::AUTO);
            if(moveWallConfComp->m_triggerType == TriggerWallMoveType_e::WALL)
            {
                GeneralCollisionComponent *genCollComp = m_ecsManager.getComponentManager().
                        searchComponentByType<GeneralCollisionComponent>(numEntity, Components_e::GENERAL_COLLISION_COMPONENT);
                assert(genCollComp);
                genCollComp->m_tagB = CollisionTag_e::TRIGGER_CT;
            }
        }
        if(memTriggerType == TriggerWallMoveType_e::BUTTON ||
                memTriggerType == TriggerWallMoveType_e::GROUND)
        {
            loadTriggerEntityData(iter->second, vectMemEntities,
                                  vectSprite, memTriggerType);
        }
    }
}

//===================================================================
void MainEngine::confBaseWallData(uint32_t wallEntity, const SpriteData &memSpriteData,
                                  const PairUI_t& coordLevel,
                                  const std::vector<uint8_t> &numWallSprites,
                                  const std::vector<SpriteData> &vectSprite, bool moveable)
{
    MemSpriteDataComponent *memSpriteComp;
    SpriteTextureComponent *spriteComp;
    TimerComponent *timerComp;
    confBaseComponent(wallEntity, memSpriteData, coordLevel,
                      CollisionShape_e::RECTANGLE_C, CollisionTag_e::WALL_CT);
    spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(wallEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    LevelCaseType_e type = moveable ? LevelCaseType_e::WALL_MOVE_LC : LevelCaseType_e::WALL_LC;
    Level::addElementCase(spriteComp, coordLevel, type, wallEntity);
    if(numWallSprites.size() == 1)
    {
        return;
    }
    memSpriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(wallEntity,
                                                          Components_e::MEM_SPRITE_DATA_COMPONENT);
    assert(memSpriteComp);
    uint32_t vectSize = numWallSprites.size();
    memSpriteComp->m_vectSpriteData.reserve(static_cast<uint32_t>(WallSpriteType_e::TOTAL_SPRITE));
    for(uint32_t j = 0; j < vectSize; ++j)
    {
        memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[numWallSprites[j]]);
    }
    timerComp = m_ecsManager.getComponentManager().
            searchComponentByType<TimerComponent>(wallEntity, Components_e::TIMER_COMPONENT);
    assert(timerComp);
    timerComp->m_clockA = std::chrono::system_clock::now();
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
            if(it->second.m_cardID)
            {
                std::string str = (*it->second.m_cardID).second;
                str.replace(0, 6, "");
                for(uint32_t k = 0; k < str.size(); ++k)
                {
                    //if char upper case add space
                    if(k != 0 && (str[k] >= 'A' && str[k] <= 'Z'))
                    {
                        str.insert(k, " ");
                        ++k;
                    }
                    str[k] = std::toupper(str[k]);
                }
                doorComp->m_cardID = {(*it->second.m_cardID).first, str};
            }
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
    float collisionRay;
    std::map<std::string, EnemyData>::const_iterator it = enemiesData.begin();
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
            enemyComp->m_life = it->second.m_life;
            enemyComp->m_visibleShot = !(it->second.m_visibleShootID.empty());
            if(it->second.m_meleeDamage)
            {
                enemyComp->m_meleeAttackDamage = *it->second.m_meleeDamage;
            }
            if(!it->second.m_dropedObjectID.empty())
            {
                enemyComp->m_dropedObjectEntity = createEnemyDropObject(levelManager, it->second, j);
            }
            if(enemyComp->m_visibleShot)
            {
                enemyComp->m_visibleAmmo.resize(4);
                confAmmoEntities(enemyComp->m_visibleAmmo, CollisionTag_e::BULLET_ENEMY_CT,
                                 enemyComp->m_visibleShot, (*it).second.m_attackPower, (*it).second.m_shotVelocity);
            }
            else
            {
                enemyComp->m_stdAmmo.resize(MAX_SHOTS);
                confAmmoEntities(enemyComp->m_stdAmmo, CollisionTag_e::BULLET_ENEMY_CT, enemyComp->m_visibleShot,
                                 (*it).second.m_attackPower);
                const MapImpactData_t &map = levelManager.getImpactDisplayData();
                MapImpactData_t::const_iterator itt = map.find(it->second.m_impactID);
                assert(itt != map.end());
                for(uint32_t j = 0; j < enemyComp->m_stdAmmo.size(); ++j)
                {
                    ShotConfComponent *shotComp = m_ecsManager.getComponentManager().
                            searchComponentByType<ShotConfComponent>(enemyComp->m_stdAmmo[j],
                                                                     Components_e::SHOT_CONF_COMPONENT);
                    assert(shotComp);
                    confShotImpactEntity(levelManager.getPictureSpriteData(), itt->second, shotComp->m_impactEntity);
                }
            }
            loadEnemySprites(levelManager.getPictureData().getSpriteData(),
                             it->second, numEntity, enemyComp, levelManager.getVisibleShootDisplayData());
            MoveableComponent *moveComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MoveableComponent>(numEntity, Components_e::MOVEABLE_COMPONENT);
            assert(moveComp);
            moveComp->m_velocity = it->second.m_velocity;
        }
    }
}

//===================================================================
void MainEngine::loadTriggerEntityData(const MoveableWallData &moveWallData,
                                       const std::vector<uint32_t> &vectPosition,
                                       const std::vector<SpriteData> &vectSprite,
                                       TriggerWallMoveType_e type)
{
    bool button = (type == TriggerWallMoveType_e::BUTTON);
    std::map<PairUI_t, uint32_t>::const_iterator it;
    PairUI_t pos;
    if(button)
    {
        assert(moveWallData.m_associatedTriggerData);
        pos = moveWallData.m_associatedTriggerData->m_pos;
        it = m_memTriggerCreated.find(pos);
    }
    else
    {
        assert(moveWallData.m_groundTriggerPos);
        pos = *moveWallData.m_groundTriggerPos;
        it = m_memTriggerCreated.find(pos);
    }
    //if trigger does not exist
    if(it == m_memTriggerCreated.end())
    {
        uint32_t numEntity = createTriggerEntity(button);
        m_memTriggerCreated.insert({pos, numEntity});
        GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().
                searchComponentByType<GeneralCollisionComponent>(numEntity, Components_e::GENERAL_COLLISION_COMPONENT);
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
                searchComponentByType<MapCoordComponent>(numEntity, Components_e::MAP_COORD_COMPONENT);
        CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
                searchComponentByType<CircleCollisionComponent>(numEntity, Components_e::CIRCLE_COLLISION_COMPONENT);
        TriggerComponent *triggerComp = m_ecsManager.getComponentManager().
                searchComponentByType<TriggerComponent>(numEntity, Components_e::TRIGGER_COMPONENT);
        assert(triggerComp);
        assert(circleComp);
        assert(mapComp);
        assert(genComp);
        if(button)
        {
            SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                    searchComponentByType<SpriteTextureComponent>(numEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
            FPSVisibleStaticElementComponent *fpsComp = m_ecsManager.getComponentManager().
                    searchComponentByType<FPSVisibleStaticElementComponent>(numEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
            assert(fpsComp);
            assert(spriteComp);
            spriteComp->m_spriteData = &vectSprite[moveWallData.m_associatedTriggerData->m_displayData.m_numSprite];
            fpsComp->m_levelElementType = LevelStaticElementType_e::GROUND;
            fpsComp->m_inGameSpriteSize = moveWallData.m_associatedTriggerData->m_displayData.m_GLSize;
            genComp->m_tagA = CollisionTag_e::WALL_CT;
            genComp->m_tagB = CollisionTag_e::TRIGGER_CT;
        }
        else
        {
            genComp->m_tagA = CollisionTag_e::TRIGGER_CT;
        }
        genComp->m_shape = CollisionShape_e::CIRCLE_C;
        circleComp->m_ray = 10.0f;
        mapComp->m_coord = pos;
        mapComp->m_absoluteMapPositionPX = getCenteredAbsolutePosition(mapComp->m_coord);
        triggerComp->m_vectElementEntities = vectPosition;
    }
    //else add new entity num to trigger
    else
    {
        TriggerComponent *triggerComp = m_ecsManager.getComponentManager().
                searchComponentByType<TriggerComponent>(it->second, Components_e::TRIGGER_COMPONENT);
        assert(triggerComp);
        for(uint32_t i = 0; i < vectPosition.size(); ++i)
        {
            triggerComp->m_vectElementEntities.push_back(vectPosition[i]);
        }
    }
}

//===================================================================
uint32_t MainEngine::createEnemyDropObject(const LevelManager &levelManager, const EnemyData &enemyData, uint32_t iterationNum)
{
    std::map<std::string, StaticLevelElementData>::const_iterator itt = levelManager.getObjectData().find(enemyData.m_dropedObjectID);
    assert(itt != levelManager.getObjectData().end());
    uint32_t objectEntity = createStaticElementEntity(LevelStaticElementType_e::OBJECT, itt->second,
                                                      levelManager.getPictureSpriteData(), iterationNum);
    GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(objectEntity, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genComp);
    genComp->m_active = false;
    return objectEntity;
}

//===================================================================
void MainEngine::createPlayerAmmoEntities(PlayerConfComponent *playerConf, CollisionTag_e collTag)
{
    WeaponComponent *weaponComp = m_ecsManager.getComponentManager().
            searchComponentByType<WeaponComponent>(
                playerConf->m_weaponEntity, Components_e::WEAPON_COMPONENT);
    assert(weaponComp);
    for(uint32_t i = 0; i < weaponComp->m_weaponsData.size(); ++i)
    {
        if(weaponComp->m_weaponsData[i].m_attackType == AttackType_e::BULLETS)
        {
            weaponComp->m_weaponsData[i].m_segmentShootEntities = std::vector<uint32_t>();
            (*weaponComp->m_weaponsData[i].m_segmentShootEntities).resize(MAX_SHOTS);
            confAmmoEntities((*weaponComp->m_weaponsData[i].m_segmentShootEntities),
                             collTag, false, weaponComp->m_weaponsData[i].m_weaponPower);
        }
    }
}

//===================================================================
void MainEngine::confAmmoEntities(std::vector<uint32_t> &ammoEntities, CollisionTag_e collTag, bool visibleShot,
                                  uint32_t damage, float shotVelocity, std::optional<float> damageRay)
{
    for(uint32_t j = 0; j < ammoEntities.size(); ++j)
    {
        ammoEntities[j] = createAmmoEntity(collTag, visibleShot);
        ShotConfComponent *shotConfComp = m_ecsManager.getComponentManager().
                searchComponentByType<ShotConfComponent>(ammoEntities[j], Components_e::SHOT_CONF_COMPONENT);
        assert(shotConfComp);
        shotConfComp->m_damage = damage;
        if(damageRay)
        {
            CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
                    searchComponentByType<CircleCollisionComponent>(ammoEntities[j], Components_e::CIRCLE_COLLISION_COMPONENT);
            assert(circleComp);
            shotConfComp->m_damageCircleRayData = createDamageZoneEntity(damage, CollisionTag_e::EXPLOSION_CT, LEVEL_TILE_SIZE_PX);
        }
        if(visibleShot)
        {
            MoveableComponent *moveComp = m_ecsManager.getComponentManager().
                    searchComponentByType<MoveableComponent>(ammoEntities[j], Components_e::MOVEABLE_COMPONENT);
            assert(moveComp);
            moveComp->m_velocity = shotVelocity;
        }
    }
}

//===================================================================
uint32_t MainEngine::createAmmoEntity(CollisionTag_e collTag, bool visibleShot)
{
    uint32_t ammoNum;
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
    genColl->m_tagA = collTag;
    genColl->m_shape = (visibleShot) ? CollisionShape_e::CIRCLE_C : CollisionShape_e::SEGMENT_C;
    if(visibleShot)
    {
        confVisibleAmmo(ammoNum);
    }
    return ammoNum;
}

//===================================================================
void MainEngine::createPlayerVisibleShotEntity(WeaponComponent *weaponConf)
{
    for(uint32_t i = 0; i < weaponConf->m_weaponsData.size(); ++i)
    {
        if(weaponConf->m_weaponsData[i].m_attackType == AttackType_e::VISIBLE_SHOTS)
        {
            weaponConf->m_weaponsData[i].m_visibleShootEntities = std::vector<uint32_t>();
            (*weaponConf->m_weaponsData[i].m_visibleShootEntities).resize(1);
            for(uint32_t j = 0; j < weaponConf->m_weaponsData[i].m_visibleShootEntities->size(); ++j)
            {
                (*weaponConf->m_weaponsData[i].m_visibleShootEntities)[j] =
                        createAmmoEntity(CollisionTag_e::BULLET_PLAYER_CT, true);
            }
        }
    }
}

//===================================================================
void MainEngine::createPlayerImpactEntities(const std::vector<SpriteData> &vectSpriteData,
                                            WeaponComponent *weaponConf,
                                            const MapImpactData_t &mapImpactData)
{
    for(uint32_t i = 0; i < weaponConf->m_weaponsData.size(); ++i)
    {
        if(weaponConf->m_weaponsData[i].m_attackType == AttackType_e::BULLETS)
        {
            MapImpactData_t::const_iterator it =
                    mapImpactData.find(weaponConf->m_weaponsData[i].m_impactID);
            assert(it != mapImpactData.end());
            assert(weaponConf->m_weaponsData[i].m_segmentShootEntities);
            for(uint32_t j = 0; j <
                weaponConf->m_weaponsData[i].m_segmentShootEntities->size(); ++j)
            {
                ShotConfComponent *shotComp = m_ecsManager.getComponentManager().
                        searchComponentByType<ShotConfComponent>((*weaponConf->m_weaponsData[i].m_segmentShootEntities)[j],
                                                                 Components_e::SHOT_CONF_COMPONENT);
                assert(shotComp);
                confShotImpactEntity(vectSpriteData, it->second, shotComp->m_impactEntity);
            }
        }
    }
}

//===================================================================
void MainEngine::confShotImpactEntity(const std::vector<SpriteData> &vectSpriteData,
                                      const PairImpactData_t &shootDisplayData,
                                      uint32_t &impactEntity)
{
    impactEntity = createShotImpactEntity();
    FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
            searchComponentByType<FPSVisibleStaticElementComponent>(
                impactEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(impactEntity,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genComp);
    assert(fpsStaticComp);
    fpsStaticComp->m_inGameSpriteSize = shootDisplayData.first[0].m_GLSize;
    fpsStaticComp->m_levelElementType = LevelStaticElementType_e::IMPACT;
    genComp->m_active = false;
    genComp->m_tagA = CollisionTag_e::IMPACT_CT;
    genComp->m_shape = CollisionShape_e::CIRCLE_C;
    loadShotImpactSprite(vectSpriteData, shootDisplayData, impactEntity);
}

//===================================================================
void MainEngine::loadEnemySprites(const std::vector<SpriteData> &vectSprite,
                                  const EnemyData &enemiesData, uint32_t numEntity,
                                  EnemyConfComponent *enemyComp, const MapVisibleShotData_t &visibleShot)
{
    MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(numEntity,
                                                          Components_e::MEM_SPRITE_DATA_COMPONENT);
    assert(memSpriteComp);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticFrontSprites, EnemySpriteType_e::STATIC_FRONT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticFrontLeftSprites, EnemySpriteType_e::STATIC_FRONT_LEFT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticFrontRightSprites, EnemySpriteType_e::STATIC_FRONT_RIGHT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticBackSprites, EnemySpriteType_e::STATIC_BACK);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticBackLeftSprites, EnemySpriteType_e::STATIC_BACK_LEFT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticBackRightSprites, EnemySpriteType_e::STATIC_BACK_RIGHT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticLeftSprites, EnemySpriteType_e::STATIC_LEFT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_staticRightSprites, EnemySpriteType_e::STATIC_RIGHT);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_attackSprites, EnemySpriteType_e::ATTACK);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_dyingSprites, EnemySpriteType_e::DYING);
    insertEnemySpriteFromType(vectSprite, enemyComp->m_mapSpriteAssociate, memSpriteComp->m_vectSpriteData,
                              enemiesData.m_touched, EnemySpriteType_e::TOUCHED);
    if(enemyComp->m_visibleShot)
    {
        loadVisibleShotData(vectSprite, enemyComp->m_visibleAmmo, enemiesData.m_visibleShootID, visibleShot);
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
void MainEngine::loadVisibleShotData(const std::vector<SpriteData> &vectSprite, const std::vector<uint32_t> &visibleAmmo,
                                     const std::string &visibleShootID, const MapVisibleShotData_t &visibleShot)
{
    for(uint32_t k = 0; k < visibleAmmo.size(); ++k)
    {
        SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                searchComponentByType<SpriteTextureComponent>(visibleAmmo[k],
                                                              Components_e::SPRITE_TEXTURE_COMPONENT);
        MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().
                searchComponentByType<MemSpriteDataComponent>(visibleAmmo[k],
                                                              Components_e::MEM_SPRITE_DATA_COMPONENT);
        FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().
                searchComponentByType<FPSVisibleStaticElementComponent>(
                    visibleAmmo[k], Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
        MemFPSGLSizeComponent *memFPSGLSizeComp = m_ecsManager.getComponentManager().
                searchComponentByType<MemFPSGLSizeComponent>(
                    visibleAmmo[k], Components_e::MEM_FPS_GLSIZE_COMPONENT);
        assert(memFPSGLSizeComp);
        assert(fpsStaticComp);
        assert(spriteComp);
        assert(memSpriteComp);
        MapVisibleShotData_t::const_iterator it = visibleShot.find(visibleShootID);
        assert(it != visibleShot.end());
        memSpriteComp->m_vectSpriteData.reserve(it->second.size());
        memFPSGLSizeComp->m_memGLSizeData.reserve(it->second.size());
        for(uint32_t l = 0; l < it->second.size(); ++l)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[it->second[l].m_numSprite]);
            memFPSGLSizeComp->m_memGLSizeData.emplace_back(it->second[l].m_GLSize);
        }
        fpsStaticComp->m_inGameSpriteSize = it->second[0].m_GLSize;
        spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[0];
    }
}

//===================================================================
void MainEngine::confVisibleAmmo(uint32_t ammoEntity)
{
    PairFloat_t pairSpriteSize = {0.2f, 0.3f};
    float collisionRay = pairSpriteSize.first * LEVEL_HALF_TILE_SIZE_PX;
    CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
            searchComponentByType<CircleCollisionComponent>(ammoEntity, Components_e::CIRCLE_COLLISION_COMPONENT);
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
uint32_t MainEngine::createDisplayTeleportEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
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
uint32_t MainEngine::createWallEntity(bool multiSprite, bool moveable)
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
    if(moveable)
    {
        bitsetComponents[Components_e::MOVEABLE_COMPONENT] = true;
        bitsetComponents[Components_e::MOVEABLE_WALL_CONF_COMPONENT] = true;
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
uint32_t MainEngine::createTriggerEntity(bool visible)
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    if(visible)
    {
        bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
        bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    }
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::TRIGGER_COMPONENT] = true;
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
    bitsetComponents[Components_e::MEM_FPS_GLSIZE_COMPONENT] = true;
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
uint32_t MainEngine::createTeleportEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::TELEPORT_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createBarrelEntity()
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
    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComponents[Components_e::MEM_FPS_GLSIZE_COMPONENT] = true;
    bitsetComponents[Components_e::BARREL_COMPONENT] = true;
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
                                   const PairUI_t& coordLevel, CollisionShape_e collisionShape,
                                   CollisionTag_e tag)
{
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(entityNum, Components_e::SPRITE_TEXTURE_COMPONENT);
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
            searchComponentByType<MapCoordComponent>(entityNum, Components_e::MAP_COORD_COMPONENT);
    assert(spriteComp);
    assert(mapComp);
    mapComp->m_coord = coordLevel;
    spriteComp->m_spriteData = &memSpriteData;
    if(tag == CollisionTag_e::WALL_CT || tag == CollisionTag_e::DOOR_CT)
    {
        mapComp->m_absoluteMapPositionPX = getAbsolutePosition(coordLevel);
    }
    else
    {
        mapComp->m_absoluteMapPositionPX = getCenteredAbsolutePosition(coordLevel);
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
    tagComp->m_tagA = tag;
}

//===================================================================
void MainEngine::confStaticComponent(uint32_t entityNum, const PairFloat_t& elementSize,
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
                                  uint32_t numWeaponEntity,
                                  uint32_t numDisplayTeleportEntity)
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
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    uint32_t entityNum = m_ecsManager.addEntity(bitsetComponents);
    confPlayerEntity(levelManager, entityNum, levelManager.getLevel(),
                     numWeaponEntity, numDisplayTeleportEntity);
    //notify player entity number
    m_graphicEngine.getMapSystem().confPlayerComp(entityNum);
    m_physicalEngine.memPlayerEntity(entityNum);
}

//===================================================================
void MainEngine::confPlayerEntity(const LevelManager &levelManager,
                                  uint32_t entityNum, const Level &level,
                                  uint32_t numWeaponEntity, uint32_t numDisplayTeleportEntity)
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
    playerConf->setIDEntityAssociated(entityNum);
    playerConf->m_displayTeleportEntity = numDisplayTeleportEntity;
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
    createPlayerAmmoEntities(m_playerConf, CollisionTag_e::BULLET_PLAYER_CT);
    createPlayerVisibleShotEntity(weaponConf);
    confPlayerVisibleShotsSprite(vectSpriteData, levelManager.getVisibleShootDisplayData(), weaponConf);
    createPlayerImpactEntities(vectSpriteData, weaponConf, levelManager.getImpactDisplayData());
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
    tagColl->m_tagA = CollisionTag_e::PLAYER_CT;
    tagColl->m_shape = CollisionShape_e::CIRCLE_C;
    //set standard weapon sprite
    StaticDisplaySystem *staticDisplay = m_ecsManager.getSystemManager().
            searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    assert(staticDisplay);
    staticDisplay->setWeaponSprite(numWeaponEntity, m_weaponComp->m_weaponsData[m_weaponComp->m_currentWeapon].m_memPosSprite.first);
    confWriteEntities();
    confMenuCursorEntity();
    confActionEntity();
    for(uint32_t i = 0; i < weaponConf->m_weaponsData.size(); ++i)
    {
        if(weaponConf->m_weaponsData[i].m_attackType == AttackType_e::MELEE)
        {
            m_playerConf->m_hitMeleeEntity = createDamageZoneEntity(weaponConf->m_weaponsData[i].m_weaponPower,
                                                                   CollisionTag_e::HIT_PLAYER_CT);
            break;
        }
    }
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
    genCollComp->m_tagA = CollisionTag_e::PLAYER_ACTION_CT;
    circleColl->m_ray = 15.0f;
    m_playerConf->m_actionEntity = entityNum;
}

//===================================================================
uint32_t MainEngine::createMeleeAttackEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::SHOT_CONF_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createDamageZoneEntity(uint32_t damage, CollisionTag_e tag, float ray)
{
    uint32_t entityNum = createMeleeAttackEntity();
    GeneralCollisionComponent *genCollComp = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(entityNum, Components_e::GENERAL_COLLISION_COMPONENT);
    CircleCollisionComponent *circleColl = m_ecsManager.getComponentManager().
            searchComponentByType<CircleCollisionComponent>(entityNum, Components_e::CIRCLE_COLLISION_COMPONENT);
    ShotConfComponent *shotComp = m_ecsManager.getComponentManager().
            searchComponentByType<ShotConfComponent>(entityNum, Components_e::SHOT_CONF_COMPONENT);
    assert(shotComp);
    assert(genCollComp);
    assert(circleColl);
    genCollComp->m_active = false;
    genCollComp->m_shape = CollisionShape_e::CIRCLE_C;
    genCollComp->m_tagA = tag;
    circleColl->m_ray = ray;
    shotComp->m_damage = damage;
    return entityNum;
}

//===================================================================
void MainEngine::loadShotImpactSprite(const std::vector<SpriteData> &vectSpriteData,
                                      const PairImpactData_t &shootDisplayData,
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
    for(uint32_t l = 0; l < shootDisplayData.first.size(); ++l)
    {
        memComp->m_vectSpriteData.emplace_back(
                    &vectSpriteData[shootDisplayData.first[l].m_numSprite]);
    }
    memComp->m_vectSpriteData.emplace_back(
                &vectSpriteData[shootDisplayData.second.m_numSprite]);
    spriteComp->m_spriteData = memComp->m_vectSpriteData[0];
}

//===================================================================
void MainEngine::confPlayerVisibleShotsSprite(const std::vector<SpriteData> &vectSpriteData,
                                              const MapVisibleShotData_t &shootDisplayData,
                                              WeaponComponent *weaponComp)
{
    for(uint32_t i = 0; i < weaponComp->m_weaponsData.size(); ++i)
    {
        if(weaponComp->m_weaponsData[i].m_visibleShootEntities)
        {
            confAmmoEntities(*weaponComp->m_weaponsData[i].m_visibleShootEntities, CollisionTag_e::BULLET_PLAYER_CT,
                             true, weaponComp->m_weaponsData[i].m_weaponPower, weaponComp->m_weaponsData[i].m_shotVelocity,
                             weaponComp->m_weaponsData[i].m_damageRay);
            loadVisibleShotData(vectSpriteData, *weaponComp->m_weaponsData[i].m_visibleShootEntities,
                                weaponComp->m_weaponsData[i].m_visibleShotID, shootDisplayData);
        }
    }
}

//===================================================================
void MainEngine::confWriteEntities()
{
    uint32_t numAmmoWrite = createWriteEntity(), numInfoWrite = createWriteEntity(),
            numLifeWrite = createWriteEntity(), numMenuWrite = createWriteEntity();
    //INFO
    WriteComponent *writeConf = m_ecsManager.getComponentManager().
            searchComponentByType<WriteComponent>(numInfoWrite, Components_e::WRITE_COMPONENT);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.95f, 0.9f};
    writeConf->m_fontSize = STD_FONT_SIZE;
    //AMMO
    writeConf = m_ecsManager.getComponentManager().
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
    m_playerConf->m_numInfoWriteEntity = numInfoWrite;
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
    posCursor->m_vertex.emplace_back(PairFloat_t{leftPos, upPos});
    posCursor->m_vertex.emplace_back(PairFloat_t{rightPos, upPos});
    posCursor->m_vertex.emplace_back(PairFloat_t{rightPos, downPos});
    posCursor->m_vertex.emplace_back(PairFloat_t{leftPos, downPos});
}

//===================================================================
void MainEngine::loadStaticElementEntities(const LevelManager &levelManager)
{
    //LOAD CURSOR MENU
    uint8_t cursorSpriteId = *levelManager.getPictureData().
            getIdentifier(levelManager.getCursorSpriteName());
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    m_memCursorSpriteData = &vectSprite[cursorSpriteId];
    loadStaticElementGroup(vectSprite, levelManager.getGroundData(), LevelStaticElementType_e::GROUND);
    loadStaticElementGroup(vectSprite, levelManager.getCeilingData(), LevelStaticElementType_e::CEILING);
    loadStaticElementGroup(vectSprite, levelManager.getObjectData(), LevelStaticElementType_e::OBJECT);
    loadStaticElementGroup(vectSprite, levelManager.getTeleportData(), LevelStaticElementType_e::TELEPORT);
    loadExitElement(levelManager, levelManager.getExitElementData());
}

//===================================================================
void MainEngine::loadBarrelElementEntities(const LevelManager &levelManager)
{
    const BarrelData &barrelData = levelManager.getBarrelData();
    for(uint32_t i = 0; i < barrelData.m_TileGamePosition.size(); ++i)
    {
        uint32_t barrelEntity = createBarrelEntity();
        SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
                searchComponentByType<SpriteTextureComponent>(barrelEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
        MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().
                searchComponentByType<MemSpriteDataComponent>(barrelEntity, Components_e::MEM_SPRITE_DATA_COMPONENT);
        GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().
                searchComponentByType<GeneralCollisionComponent>(barrelEntity, Components_e::GENERAL_COLLISION_COMPONENT);
        CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
                searchComponentByType<CircleCollisionComponent>(barrelEntity, Components_e::CIRCLE_COLLISION_COMPONENT);
        FPSVisibleStaticElementComponent *fpsComp = m_ecsManager.getComponentManager().
                searchComponentByType<FPSVisibleStaticElementComponent>(barrelEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
        BarrelComponent *barrelComp = m_ecsManager.getComponentManager().
                searchComponentByType<BarrelComponent>(barrelEntity, Components_e::BARREL_COMPONENT);
        MemFPSGLSizeComponent *memGLSizeComp = m_ecsManager.getComponentManager().
                searchComponentByType<MemFPSGLSizeComponent>(barrelEntity, Components_e::MEM_FPS_GLSIZE_COMPONENT);
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().
                searchComponentByType<MapCoordComponent>(barrelEntity, Components_e::MAP_COORD_COMPONENT);
        TimerComponent *timerComp = m_ecsManager.getComponentManager().
                searchComponentByType<TimerComponent>(barrelEntity, Components_e::TIMER_COMPONENT);
        assert(timerComp);
        assert(mapComp);
        assert(memGLSizeComp);
        assert(barrelComp);
        assert(genComp);
        assert(fpsComp);
        assert(circleComp);
        assert(memSpriteComp);
        assert(spriteComp);
        mapComp->m_coord = barrelData.m_TileGamePosition[i];
        mapComp->m_absoluteMapPositionPX = getCenteredAbsolutePosition(mapComp->m_coord);
        circleComp->m_ray = 10.0f;
        genComp->m_tagA = CollisionTag_e::BARREL_CT;
        genComp->m_tagB = CollisionTag_e::BARREL_CT;
        genComp->m_shape = CollisionShape_e::CIRCLE_C;
        uint32_t totalSize = barrelData.m_staticSprite.size() + barrelData.m_explosionSprite.size();
        memGLSizeComp->m_memGLSizeData.reserve(totalSize);
        memSpriteComp->m_vectSpriteData.reserve(totalSize);
        memSpriteComp->m_current = 0;
        const std::vector<SpriteData> &spriteData = levelManager.getPictureData().getSpriteData();
        for(uint32_t j = 0; j < barrelData.m_staticSprite.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&spriteData[barrelData.m_staticSprite[j]]);
            memGLSizeComp->m_memGLSizeData.emplace_back(barrelData.m_inGameStaticSpriteSize);
        }
        for(uint32_t j = 0; j < barrelData.m_explosionSprite.size(); ++j)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&spriteData[barrelData.m_explosionSprite[j]]);
            memGLSizeComp->m_memGLSizeData.emplace_back(barrelData.m_vectinGameExplosionSpriteSize[j]);
        }
        spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[0];
        fpsComp->m_inGameSpriteSize = memGLSizeComp->m_memGLSizeData[0];
        fpsComp->m_levelElementType = LevelStaticElementType_e::GROUND;
        barrelComp->m_phaseDestructPhaseNumber = barrelData.m_explosionSprite.size();
        barrelComp->m_life = 3;
        barrelComp->m_memPosExplosionSprite = barrelData.m_staticSprite.size() - 1;
        barrelComp->m_damageZoneEntity = createDamageZoneEntity(15, CollisionTag_e::EXPLOSION_CT, 30.0f);
        timerComp->m_clockA = std::chrono::system_clock::now();
    }
}

//===================================================================
uint32_t MainEngine::loadDisplayTeleportEntity(const LevelManager &levelManager)
{
    uint32_t numEntity = createDisplayTeleportEntity();
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(numEntity,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<MemSpriteDataComponent>(numEntity,
                                                          Components_e::MEM_SPRITE_DATA_COMPONENT);
    GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().
            searchComponentByType<GeneralCollisionComponent>(numEntity,
                                                             Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genComp);
    CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
            searchComponentByType<CircleCollisionComponent>(numEntity,
                                                            Components_e::CIRCLE_COLLISION_COMPONENT);
    FPSVisibleStaticElementComponent *fpsComp = m_ecsManager.getComponentManager().
            searchComponentByType<FPSVisibleStaticElementComponent>(numEntity,
                                                                    Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(fpsComp);
    assert(circleComp);
    assert(memSpriteComp);
    assert(spriteComp);
    circleComp->m_ray = 10.0f;
    genComp->m_tagA = CollisionTag_e::GHOST_CT;
    genComp->m_tagB = CollisionTag_e::TELEPORT_ANIM_CT;
    genComp->m_shape = CollisionShape_e::CIRCLE_C;
    genComp->m_active = false;
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    const std::vector<MemSpriteData> &visibleTeleportData = levelManager.getVisibleTeleportData();
    memSpriteComp->m_vectSpriteData.reserve(visibleTeleportData.size());
    fpsComp->m_inGameSpriteSize = visibleTeleportData[0].m_GLSize;
    fpsComp->m_levelElementType = LevelStaticElementType_e::GROUND;
    for(uint32_t j = 0; j < visibleTeleportData.size(); ++j)
    {
        memSpriteComp->m_current = 0;
        memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[visibleTeleportData[j].m_numSprite]);
    }
    spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[0];
    return numEntity;
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
void MainEngine::loadStaticElementGroup(const std::vector<SpriteData> &vectSpriteData,
                                        const std::map<std::string, StaticLevelElementData> &staticData,
                                        LevelStaticElementType_e elementType)
{
    std::map<std::string, StaticLevelElementData>::const_iterator it = staticData.begin();
    for(; it != staticData.end(); ++it)
    {
        for(uint32_t j = 0; j < it->second.m_TileGamePosition.size(); ++j)
        {
            createStaticElementEntity(elementType, it->second, vectSpriteData, j);
        }
    }
}

//===================================================================
uint32_t MainEngine::createStaticElementEntity(LevelStaticElementType_e elementType, const StaticLevelElementData &staticElementData,
                                               const std::vector<SpriteData> &vectSpriteData, uint32_t iterationNum)
{
    CollisionTag_e tag;
    uint32_t entityNum;
    const SpriteData &memSpriteData = vectSpriteData[staticElementData.m_numSprite];
    float collisionRay = staticElementData.m_inGameSpriteSize.first * LEVEL_HALF_TILE_SIZE_PX;
    if(elementType == LevelStaticElementType_e::OBJECT)
    {
        tag = CollisionTag_e::OBJECT_CT;
        entityNum = confObjectEntity(staticElementData);
    }
    else if(elementType == LevelStaticElementType_e::TELEPORT)
    {
        tag = CollisionTag_e::TELEPORT_CT;
        entityNum = confTeleportEntity(staticElementData, iterationNum);
    }
    else
    {
        if(staticElementData.m_traversable)
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
            searchComponentByType<FPSVisibleStaticElementComponent>(entityNum, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(fpsStaticComp);
    fpsStaticComp->m_inGameSpriteSize = staticElementData.m_inGameSpriteSize;
    confBaseComponent(entityNum, memSpriteData, staticElementData.m_TileGamePosition[iterationNum], CollisionShape_e::CIRCLE_C, tag);
    CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().
            searchComponentByType<CircleCollisionComponent>(entityNum, Components_e::CIRCLE_COLLISION_COMPONENT);
    assert(circleComp);
    circleComp->m_ray = collisionRay;
    confStaticComponent(entityNum, staticElementData.m_inGameSpriteSize, elementType);
    return entityNum;
}

//===================================================================
uint32_t MainEngine::confTeleportEntity(const StaticLevelElementData &teleportData, uint32_t iterationNum)
{
    uint32_t entityNum = createTeleportEntity();
    TeleportComponent *teleportComp = m_ecsManager.getComponentManager().
            searchComponentByType<TeleportComponent>(entityNum, Components_e::TELEPORT_COMPONENT);
    assert(teleportComp);
    teleportComp->m_targetPos = teleportData.m_teleportData->m_targetTeleport[iterationNum];
    return entityNum;
}

//===================================================================
uint32_t MainEngine::confObjectEntity(const StaticLevelElementData &objectData)
{
    uint32_t entityNum = createObjectEntity();
    ObjectConfComponent *objComp = m_ecsManager.getComponentManager().
            searchComponentByType<ObjectConfComponent>(entityNum, Components_e::OBJECT_CONF_COMPONENT);
    assert(objComp);
    objComp->m_type = objectData.m_type;
    if(objComp->m_type == ObjectType_e::AMMO_WEAPON || objComp->m_type == ObjectType_e::WEAPON ||
            objComp->m_type == ObjectType_e::HEAL)
    {
        objComp->m_containing = objectData.m_containing;
        objComp->m_weaponID = objectData.m_weaponID;
    }
    else if(objComp->m_type == ObjectType_e::CARD)
    {
        objComp->m_cardID = objectData.m_cardID;
    }
    return entityNum;
}


//===================================================================
void MainEngine::confColorBackgroundComponents(uint32_t entity, const GroundCeilingData &groundData, bool ground)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entity, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex.reserve(4);
    if(ground)
    {
        posComp->m_vertex.emplace_back(-1.0f, 0.0f);
        posComp->m_vertex.emplace_back(1.0f, 0.0f);
        posComp->m_vertex.emplace_back(1.0f, -1.0f);
        posComp->m_vertex.emplace_back(-1.0f, -1.0f);
    }
    else
    {
        posComp->m_vertex.emplace_back(-1.0f, 1.0f);
        posComp->m_vertex.emplace_back(1.0f, 1.0f);
        posComp->m_vertex.emplace_back(1.0f, 0.0f);
        posComp->m_vertex.emplace_back(-1.0f, 0.0f);
    }
    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().
            searchComponentByType<ColorVertexComponent>(entity, Components_e::COLOR_VERTEX_COMPONENT);
    assert(colorComp);
    colorComp->m_vertex.reserve(4);
    colorComp->m_vertex.emplace_back(std::get<0>(groundData.m_color[0]), std::get<1>(groundData.m_color[0]),
            std::get<2>(groundData.m_color[0]), 1.0);
    colorComp->m_vertex.emplace_back(std::get<0>(groundData.m_color[1]), std::get<1>(groundData.m_color[1]),
            std::get<2>(groundData.m_color[1]), 1.0);
    colorComp->m_vertex.emplace_back(std::get<0>(groundData.m_color[2]), std::get<1>(groundData.m_color[2]),
            std::get<2>(groundData.m_color[2]), 1.0);
    colorComp->m_vertex.emplace_back(std::get<0>(groundData.m_color[3]), std::get<1>(groundData.m_color[3]),
            std::get<2>(groundData.m_color[3]), 1.0);
}

//===================================================================
void MainEngine::confGroundSimpleTextBackgroundComponents(uint32_t entity, const GroundCeilingData &groundData,
                                                          const std::vector<SpriteData> &vectSprite)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entity, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex.reserve(6);
    posComp->m_vertex.emplace_back(-1.0f, 0.0f);
    posComp->m_vertex.emplace_back(1.0f, 0.0f);
    posComp->m_vertex.emplace_back(1.0f, -1.0f);
    posComp->m_vertex.emplace_back(-1.0f, -1.0f);
    posComp->m_vertex.emplace_back(3.0f, 0.0f);
    posComp->m_vertex.emplace_back(3.0f, -1.0f);
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(entity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    assert(vectSprite.size() >= groundData.m_spriteSimpleTextNum);
    spriteComp->m_spriteData = &vectSprite[groundData.m_spriteSimpleTextNum];
}

//===================================================================
void MainEngine::confCeilingSimpleTextBackgroundComponents(uint32_t entity, const GroundCeilingData &groundData,
                                                           const std::vector<SpriteData> &vectSprite)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().
            searchComponentByType<PositionVertexComponent>(entity, Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    posComp->m_vertex.reserve(6);
    posComp->m_vertex.emplace_back(-1.0f, 1.0f);
    posComp->m_vertex.emplace_back(1.0f, 1.0f);
    posComp->m_vertex.emplace_back(1.0f, 0.0f);
    posComp->m_vertex.emplace_back(-1.0f, 0.0f);
    posComp->m_vertex.emplace_back(3.0f, 1.0f);
    posComp->m_vertex.emplace_back(3.0f, 0.0f);
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(entity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    assert(vectSprite.size() >= groundData.m_spriteSimpleTextNum);
    spriteComp->m_spriteData = &vectSprite[groundData.m_spriteSimpleTextNum];
}

//===================================================================
void MainEngine::confTiledTextBackgroundComponents(uint32_t entity, const GroundCeilingData &backgroundData,
                                                   const std::vector<SpriteData> &vectSprite)
{
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().
            searchComponentByType<SpriteTextureComponent>(entity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    assert(vectSprite.size() >= backgroundData.m_spriteTiledTextNum);
    spriteComp->m_spriteData = &vectSprite[backgroundData.m_spriteTiledTextNum];
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
    DoorWallSystem *door = m_ecsManager.getSystemManager().
            searchSystemByType<DoorWallSystem>(static_cast<uint32_t>(Systems_e::DOOR_SYSTEM));
    IASystem *iaSystem = m_ecsManager.getSystemManager().
            searchSystemByType<IASystem>(static_cast<uint32_t>(Systems_e::IA_SYSTEM));
    input->setGLWindow(m_graphicEngine.getGLWindow());
    input->linkMainEngine(this);
    iaSystem->linkMainEngine(this);
    m_physicalEngine.linkSystems(input, coll, door, iaSystem);
}
