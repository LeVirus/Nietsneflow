#include "MainEngine.hpp"
#include "Game.hpp"
#include "constants.hpp"
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ImpactShotComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/WallMultiSpriteConf.hpp>
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
#include <ECS/Components/CheckpointComponent.hpp>
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
#include <ECS/Components/LogComponent.hpp>
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
void MainEngine::init(Game *refGame)
{
    std::srand(std::time(nullptr));
    m_ecsManager.init();
    linkSystemsToGraphicEngine();
    linkSystemsToPhysicalEngine();
    linkSystemsToSoundEngine();
    m_audioEngine.initOpenAL();
    m_refGame = refGame;
}

//===================================================================
LevelState MainEngine::displayTitleMenu(const LevelManager &levelManager)
{
    uint16_t genericBackgroundMenuSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getGenericMenuSpriteName()),
            titleBackgroundMenuSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getTitleMenuSpriteName()),
            leftBackgroundMenuSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getLeftMenuSpriteName()),
            rightLeftBackgroundMenuSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getRightLeftMenuSpriteName());
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    m_memBackgroundGenericMenu = &vectSprite[genericBackgroundMenuSpriteId];
    m_memBackgroundTitleMenu = &vectSprite[titleBackgroundMenuSpriteId];
    m_memBackgroundLeftMenu = &vectSprite[leftBackgroundMenuSpriteId];
    m_memBackgroundRightLeftMenu = &vectSprite[rightLeftBackgroundMenuSpriteId];
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    playerConf.m_menuMode = MenuMode_e::TITLE;
    setMenuEntries(playerConf);
    m_gamePaused = true;
    m_titleMenuMode = true;
    //prevent to exit
    m_currentLevelState = LevelState_e::EXIT;
    //game paused
    do
    {
        m_graphicEngine.runIteration(m_gamePaused);
        m_physicalEngine.runIteration(m_gamePaused);
        if(m_graphicEngine.windowShouldClose())
        {
            return {LevelState_e::EXIT, 0, false};
        }
        if(m_physicalEngine.toogledFullScreenSignal())
        {
            m_graphicEngine.toogleFullScreen();
            validDisplayMenu();
            m_physicalEngine.reinitToggleFullScreen();
        }
    }while(m_currentLevelState != LevelState_e::NEW_GAME && m_currentLevelState != LevelState_e::LOAD_GAME);
    m_titleMenuMode = false;
    uint32_t levelToLoad = m_levelToLoad->first;
    m_levelToLoad = {};
    return {m_currentLevelState, levelToLoad, playerConf.m_previousMenuMode == MenuMode_e::LOAD_CUSTOM_LEVEL};
}

//===================================================================
LevelState MainEngine::mainLoop(uint32_t levelNum, LevelState_e levelState, bool afterLoadFailure, bool customLevel)
{
    m_levelEnd = false;
    m_currentLevelState = levelState;
    m_currentLevel = levelNum;
    m_graphicEngine.getMapSystem().confLevelData();
    m_physicalEngine.updateMousePos();
    if(!afterLoadFailure)
    {
        initLevel(levelNum, levelState);
        if(!m_graphicEngine.prologueEmpty() && !m_memCheckpointData)
        {
            displayTransitionMenu(MenuMode_e::LEVEL_PROLOGUE);
        }
        if(levelState != LevelState_e::LOAD_GAME)
        {
            if(levelState == LevelState_e::RESTART_FROM_CHECKPOINT || (levelState == LevelState_e::GAME_OVER && m_memCheckpointData))
            {
                assert(m_memCheckpointData);
                assert(m_currentSave);
            }
            else
            {
                if(levelState == LevelState_e::RESTART_LEVEL)
                {
                    m_graphicEngine.setRestartLevelMode();
                }
                saveGameProgress(levelNum);
            }
        }
    }
    std::chrono::duration<double> elapsed_seconds/*, fps*/;
    m_graphicEngine.unsetTransition(m_gamePaused);
    std::chrono::time_point<std::chrono::system_clock> clock/*, clockFrame*/;
    clock = std::chrono::system_clock::now();
    m_physicalEngine.updateMousePos();
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    do
    {
        elapsed_seconds = std::chrono::system_clock::now() - clock;
        if(FPS_VALUE > elapsed_seconds.count())
        {
            continue;
        }
        //display FPS
//        fps = std::chrono::system_clock::now() - clockFrame;
//        std::cout << 1.0f / fps.count() << "  " << fps.count() << " FPS\n";
//        clockFrame = std::chrono::system_clock::now();
        clock = std::chrono::system_clock::now();
        m_physicalEngine.runIteration(m_gamePaused);
        //LOAD if level to load break the loop
        if(m_levelToLoad)
        {
            if(!m_memCheckpointLevelState)
            {
                m_memEnemiesStateFromCheckpoint.clear();
            }
            uint32_t levelToLoad = m_levelToLoad->first;
            bool customLevelMode = m_levelToLoad->second;
            m_levelToLoad = {};
            if(m_currentLevelState == LevelState_e::NEW_GAME || m_currentLevelState == LevelState_e::RESTART_LEVEL)
            {
                if(m_currentLevelState == LevelState_e::NEW_GAME)
                {
                    m_playerMemGear = false;
                }
                clearCheckpointData();
            }
            return {m_currentLevelState, levelToLoad, customLevelMode};
        }
        clearObjectToDelete();
        if(m_physicalEngine.toogledFullScreenSignal())
        {
            m_graphicEngine.toogleFullScreen();
            validDisplayMenu();
            m_physicalEngine.reinitToggleFullScreen();
        }
        m_graphicEngine.runIteration(m_gamePaused);
        //MUUUUUUUUUUUUSSSSS
        m_audioEngine.runIteration();
        if(playerConf.m_checkpointReached)
        {
            //MEM ENTITIES TO DELETE WHEN CHECKPOINT IS REACHED
            m_currentEntitiesDelete = m_memStaticEntitiesDeletedFromCheckpoint;
            saveGameProgressCheckpoint(levelNum, *playerConf.m_checkpointReached, *playerConf.m_currentCheckpoint);
            playerConf.m_checkpointReached = {};
        }
        //level end
        if(m_levelEnd)
        {
            m_currentLevelState = LevelState_e::LEVEL_END;
            clearCheckpointData();
            //end level
            playerConf.m_inMovement = false;
            playerConf.m_infoWriteData = {false, ""};
            savePlayerGear(true);
            m_graphicEngine.setTransition(m_gamePaused);
            displayTransitionMenu();
            if(!m_graphicEngine.epilogueEmpty())
            {
                displayTransitionMenu(MenuMode_e::LEVEL_EPILOGUE);
            }
            m_memEnemiesStateFromCheckpoint.clear();
            return {m_currentLevelState, {}, customLevel};
        }
        //Player dead
        else if(!playerConf.m_life)
        {
            playerConf.m_infoWriteData = {false, ""};
            AudioComponent *audioComp = m_ecsManager.getComponentManager().getAudioComponent(m_playerEntity);
            assert(audioComp);
            //play death sound
            audioComp->m_soundElements[1]->m_toPlay = true;
            m_audioEngine.getSoundSystem()->execSystem();
            if(!m_memCheckpointLevelState)
            {
                clearCheckpointData();
            }
            //display red transition
            m_graphicEngine.setTransition(m_gamePaused, true);
            displayTransitionMenu(MenuMode_e::TRANSITION_LEVEL, true);
            return {LevelState_e::GAME_OVER, {}, customLevel};
        }
    }while(!m_graphicEngine.windowShouldClose());
    return {LevelState_e::EXIT, {}, customLevel};
}

//===================================================================
void MainEngine::initLevel(uint32_t levelNum, LevelState_e levelState)
{
    bool beginLevel = isLoadFromLevelBegin(m_currentLevelState);
    if(beginLevel)
    {
        m_memCheckpointLevelState = std::nullopt;
        m_ecsManager.getComponentManager().getPlayerConfComp().m_currentCheckpoint->first = 0;
        if(levelState == LevelState_e::NEW_GAME)
        {
            m_graphicEngine.updateSaveNum(levelNum, m_currentSave, 0, "", true);
        }
        else if(!m_memCustomLevelLoadedData)
        {
            m_graphicEngine.updateSaveNum(levelNum, m_currentSave, {}, "", true);
        }
    }
    //don't load gear for custom level
    if(!m_memCustomLevelLoadedData)
    {
        if(m_playerMemGear)
        {
            loadPlayerGear(beginLevel);
        }
        else
        {
            savePlayerGear(beginLevel);
            saveGameProgress(m_currentLevel);
        }
    }
    //display FPS
    //    std::chrono::duration<double> fps;
    //    std::chrono::time_point<std::chrono::system_clock> clockFrame  = std::chrono::system_clock::now();
    if(m_currentLevelState == LevelState_e::NEW_GAME || m_currentLevelState == LevelState_e::LOAD_GAME ||
            m_currentLevelState == LevelState_e::RESTART_LEVEL || m_currentLevelState == LevelState_e::RESTART_FROM_CHECKPOINT)
    {
        m_vectMemPausedTimer.clear();
        if(m_gamePaused)
        {
            setUnsetPaused();
        }
    }
    if(m_memCheckpointLevelState)
    {
        loadGameProgressCheckpoint();
    }
    else
    {
        m_memStaticEntitiesDeletedFromCheckpoint.clear();
        m_currentEntitiesDelete.clear();
    }
}

//===================================================================
void MainEngine::saveGameProgressCheckpoint(uint32_t levelNum, const PairUI_t &checkpointReached,
                                            const std::pair<uint32_t, Direction_e> &checkpointData)
{
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    uint32_t enemiesKilled = (playerConf.m_enemiesKilled) ? *playerConf.m_enemiesKilled : 0;
    uint32_t secretsFound = (playerConf.m_secretsFound) ? *playerConf.m_secretsFound : 0;
    m_memCheckpointLevelState = {levelNum, checkpointData.first, secretsFound, enemiesKilled, checkpointData.second,
                                 checkpointReached};
    //OOOK SAVE GEAR BEGIN LEVEL
    savePlayerGear(false);
    saveEnemiesCheckpoint();
    std::vector<PairUI_t> revealedMap;
    const std::map<uint32_t, PairUI_t> &map = m_graphicEngine.getMapSystem().getDetectedMapData();
    revealedMap.reserve(map.size());
    for(std::map<uint32_t, PairUI_t>::const_iterator it = map.begin(); it != map.end(); ++it)
    {
        revealedMap.emplace_back(it->second);
    }
    m_memCheckpointData = {checkpointData.first, secretsFound, enemiesKilled, checkpointReached,
                           checkpointData.second, m_memEnemiesStateFromCheckpoint,
                           m_memMoveableWallCheckpointData, m_memTriggerWallMoveableWallCheckpointData,
                           m_memStaticEntitiesDeletedFromCheckpoint, revealedMap, playerConf.m_card};
    saveGameProgress(m_currentLevel, m_currentSave, &(*m_memCheckpointData));
}

//===================================================================
void MainEngine::saveEnemiesCheckpoint()
{
    std::vector<MemCheckpointEnemiesState> vectEnemiesData;
    vectEnemiesData.reserve(m_memEnemiesStateFromCheckpoint.size());
    for(uint32_t i = 0; i < m_memEnemiesStateFromCheckpoint.size(); ++i)
    {
        EnemyConfComponent *enemyComp = m_ecsManager.getComponentManager().getEnemyConfComponent(m_memEnemiesStateFromCheckpoint[i].m_entityNum);
        assert(enemyComp);
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(m_memEnemiesStateFromCheckpoint[i].m_entityNum);
        assert(mapComp);
        m_memEnemiesStateFromCheckpoint[i].m_dead = (enemyComp->m_behaviourMode == EnemyBehaviourMode_e::DEAD ||
                                                     enemyComp->m_behaviourMode == EnemyBehaviourMode_e::DYING);
        m_memEnemiesStateFromCheckpoint[i].m_enemyPos = mapComp->m_absoluteMapPositionPX;
        m_memEnemiesStateFromCheckpoint[i].m_objectPickedUp = false;
        m_memEnemiesStateFromCheckpoint[i].m_life = enemyComp->m_life;
        if(enemyComp->m_dropedObjectEntity)
        {
            //check if entity still exists
            GeneralCollisionComponent *genCompObject = m_ecsManager.getComponentManager().
                                                       getGeneralCollisionComponent(*enemyComp->m_dropedObjectEntity);
            if(!genCompObject)
            {
                m_memEnemiesStateFromCheckpoint[i].m_objectPickedUp = true;
            }
        }
    }
}

//===================================================================
void MainEngine::saveGameProgress(uint32_t levelNum, std::optional<uint32_t> numSaveFile,
                                  const MemCheckpointElementsState *checkpointData)
{
    uint32_t saveNum = numSaveFile ? *numSaveFile : m_currentSave;
    if(!checkpointData)
    {
        m_memCheckpointLevelState = std::nullopt;
        if(m_memCustomLevelLoadedData)
        {
            m_memCustomLevelLoadedData->m_playerConfCheckpoint = m_memPlayerConfBeginLevel;
        }
    }
    if(checkpointData)
    {
        if(!m_memCustomLevelLoadedData)
        {
            m_graphicEngine.updateGraphicCheckpointData(checkpointData, saveNum);
        }
        else
        {
            m_memCustomLevelLoadedData->m_checkpointLevelData = *checkpointData;
            m_memCustomLevelLoadedData->m_playerConfCheckpoint = m_memPlayerConfCheckpoint;
            memCustomLevelRevealedMap();
        }
    }
    if(!m_memCustomLevelLoadedData)
    {
        std::string date = m_refGame->saveGameProgressINI(m_memPlayerConfBeginLevel, m_memPlayerConfCheckpoint,
                                                          levelNum, saveNum, checkpointData);
        m_graphicEngine.updateSaveNum(levelNum, saveNum, {}, date);
    }
}

//===================================================================
void MainEngine::memCustomLevelRevealedMap()
{
    const std::map<uint32_t, PairUI_t> &revealedMap = m_graphicEngine.getMapSystem().getRevealedMap();
    m_revealedMapData.clear();
    m_revealedMapData.reserve(revealedMap.size());
    for(std::map<uint32_t, PairUI_t>::const_iterator it = revealedMap.begin(); it != revealedMap.end(); ++it)
    {
        m_revealedMapData.emplace_back(it->second);
    }
}

//===================================================================
void MainEngine::savePlayerGear(bool beginLevel)
{
    if(m_memCustomLevelLoadedData)
    {
        return;
    }
    PlayerConfComponent &playerConfComp = m_ecsManager.getComponentManager().getPlayerConfComp();
    WeaponComponent *weaponConf = m_ecsManager.getComponentManager().getWeaponComponent(
        playerConfComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)]);
    assert(weaponConf);
    MemPlayerConf &playerConf = beginLevel ? m_memPlayerConfBeginLevel : m_memPlayerConfCheckpoint;
    playerConf.m_ammunationsCount.resize(weaponConf->m_weaponsData.size());
    playerConf.m_weapons.resize(weaponConf->m_weaponsData.size());
    for(uint32_t i = 0; i < playerConf.m_ammunationsCount.size(); ++i)
    {
        playerConf.m_ammunationsCount[i] =
                weaponConf->m_weaponsData[i].m_ammunationsCount;
        playerConf.m_weapons[i] = weaponConf->m_weaponsData[i].m_posses;
    }
    playerConf.m_currentWeapon = weaponConf->m_currentWeapon;
    playerConf.m_previousWeapon = weaponConf->m_previousWeapon;
    playerConf.m_life = playerConf.m_life;
    m_playerMemGear = true;
    if(beginLevel)
    {
        m_memCheckpointLevelState = std::nullopt;
    }
}

//===================================================================
void MainEngine::unsetFirstLaunch()
{
    m_ecsManager.getComponentManager().getPlayerConfComp().m_firstMenu = false;
}

//===================================================================
void MainEngine::clearMemSoundElements()
{
    m_memSoundElements.m_teleports = std::nullopt;
    m_memSoundElements.m_enemies = std::nullopt;
    m_memSoundElements.m_barrels = std::nullopt;
    m_memSoundElements.m_damageZone = std::nullopt;
    m_memSoundElements.m_visibleShots = std::nullopt;
}

//===================================================================
void MainEngine::loadPlayerGear(bool beginLevel)
{
    PlayerConfComponent &playerConfComp = m_ecsManager.getComponentManager().getPlayerConfComp();
    WeaponComponent *weaponConf = m_ecsManager.getComponentManager().getWeaponComponent(
        playerConfComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)]);
    assert(weaponConf);
    MemPlayerConf &playerConf = beginLevel ? m_memPlayerConfBeginLevel : m_memPlayerConfCheckpoint;
    assert(playerConf.m_ammunationsCount.size() == weaponConf->m_weaponsData.size());
    for(uint32_t i = 0; i < playerConf.m_ammunationsCount.size(); ++i)
    {
        weaponConf->m_weaponsData[i].m_ammunationsCount = playerConf.m_ammunationsCount[i];
        weaponConf->m_weaponsData[i].m_posses = playerConf.m_weapons[i];
    }
    weaponConf->m_currentWeapon = playerConf.m_currentWeapon;
    weaponConf->m_previousWeapon = playerConf.m_previousWeapon;
    playerConfComp.m_life = playerConf.m_life;
    StaticDisplaySystem *staticDisplay = m_ecsManager.getSystemManager().
            searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    assert(staticDisplay);
    //update FPS weapon sprite
    //weapon type weapon sprite
    staticDisplay->setWeaponSprite(playerConfComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)],
            weaponConf->m_weaponsData[weaponConf->m_currentWeapon].m_memPosSprite.first);
}

//===================================================================
void MainEngine::displayTransitionMenu(MenuMode_e mode, bool redTransition)
{
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    float topEpiloguePosition;
    playerConf.m_menuMode = mode;
    setMenuEntries(playerConf);
    WriteComponent *writeConf = m_ecsManager.getComponentManager().getWriteComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::TITLE_MENU)]);
    assert(writeConf);
    if(mode == MenuMode_e::LEVEL_EPILOGUE)
    {
        topEpiloguePosition = getTopEpilogueVerticalPosition(writeConf);
        writeConf->m_upLeftPositionGL.second = -1.0f;
        m_audioEngine.playEpilogueMusic();
    }
    m_gamePaused = true;
    m_physicalEngine.setModeTransitionMenu(true);
    m_graphicEngine.mainDisplay(m_gamePaused);
    playerConf.m_currentCursorPos = 0;
    m_graphicEngine.unsetTransition(m_gamePaused, redTransition);
    do
    {
        m_graphicEngine.runIteration(m_gamePaused);
        m_physicalEngine.runIteration(m_gamePaused);
        if(mode == MenuMode_e::LEVEL_EPILOGUE)
        {
            writeConf->m_upLeftPositionGL.second += 0.005f;
            if(writeConf->m_upLeftPositionGL.second > topEpiloguePosition)
            {
                m_gamePaused = false;
            }
        }
    }while(m_gamePaused);
    m_physicalEngine.setModeTransitionMenu(false);
    m_graphicEngine.setTransition(true);
    if(playerConf.m_menuMode == MenuMode_e::TRANSITION_LEVEL)
    {
        m_graphicEngine.fillMenuWrite(*writeConf, MenuMode_e::BASE);
    }
}

//===================================================================
float getTopEpilogueVerticalPosition(const WriteComponent *writeComp)
{
    uint32_t nbLine = 1;
    std::string::size_type pos = 0;
    do
    {
        pos = writeComp->m_vectMessage[0].second.find('\\', ++pos);
        if(pos != std::string::npos)
        {
            ++nbLine;
        }
        else
        {
            break;
        }
    }while(true);
    //base position + line y size * line number
    return 1.0f + (writeComp->m_fontSize * 2.0f) * nbLine;
}

//===================================================================
void MainEngine::confPlayerVisibleShoot(std::vector<uint32_t> &playerVisibleShots,
                                        const PairFloat_t &point, float degreeAngle)
{
    m_physicalEngine.confPlayerVisibleShoot(playerVisibleShots, point, degreeAngle);
}

//===================================================================
void MainEngine::playerAttack(uint32_t playerEntity, PlayerConfComponent &playerComp, const PairFloat_t &point, float degreeAngle)
{
    PlayerConfComponent &playerConfComp = m_ecsManager.getComponentManager().getPlayerConfComp();
    WeaponComponent *weaponConf = m_ecsManager.getComponentManager().getWeaponComponent(
        playerConfComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)]);
    assert(weaponConf);
    assert(weaponConf->m_currentWeapon < weaponConf->m_weaponsData.size());
    WeaponData &currentWeapon = weaponConf->m_weaponsData[
            weaponConf->m_currentWeapon];
    AttackType_e attackType = currentWeapon.m_attackType;
    if(attackType == AttackType_e::MELEE)
    {
        GeneralCollisionComponent *actionGenColl = m_ecsManager.getComponentManager().getGeneralCollisionComponent(
            playerComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::HIT_MELEE)]);
        MoveableComponent *playerMoveComp = m_ecsManager.getComponentManager().getMoveableComponent(playerEntity);
        MapCoordComponent *actionMapComp = m_ecsManager.getComponentManager().getMapCoordComponent(
            playerComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::HIT_MELEE)]);
        MapCoordComponent *playerMapComp = m_ecsManager.getComponentManager().getMapCoordComponent(playerEntity);
        assert(actionMapComp);
        assert(playerMoveComp);
        assert(actionGenColl);
        assert(playerMapComp);
        confActionShape(*actionMapComp, *actionGenColl, *playerMapComp, *playerMoveComp);
        return;
    }
    else if(attackType == AttackType_e::BULLETS)
    {
        if(currentWeapon.m_simultaneousShots == 1)
        {
            confPlayerBullet(&playerComp, point, degreeAngle, currentWeapon.m_currentBullet);
            if(currentWeapon.m_currentBullet < MAX_SHOTS - 1)
            {
                ++currentWeapon.m_currentBullet;
            }
            else
            {
                currentWeapon.m_currentBullet = 0;
            }
        }
        else
        {
            for(uint32_t i = 0; i < currentWeapon.m_simultaneousShots; ++i)
            {
                confPlayerBullet(&playerComp, point, degreeAngle, i);
            }
        }
    }
    else if(attackType == AttackType_e::VISIBLE_SHOTS)
    {
        assert(currentWeapon.m_visibleShootEntities);
        confPlayerVisibleShoot((*currentWeapon.m_visibleShootEntities), point, degreeAngle);
    }
    assert(weaponConf->m_weaponsData[weaponConf->m_currentWeapon].m_ammunationsCount > 0);
    --weaponConf->m_weaponsData[weaponConf->m_currentWeapon].m_ammunationsCount;
}

//===================================================================
void MainEngine::confPlayerBullet(PlayerConfComponent *playerComp,
                                  const PairFloat_t &point, float degreeAngle,
                                  uint32_t numBullet)
{
    assert(numBullet < MAX_SHOTS);
    WeaponComponent *weaponComp = m_ecsManager.getComponentManager().getWeaponComponent(
        playerComp->m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)]);
    assert(weaponComp);
    uint32_t bulletEntity = (*weaponComp->m_weaponsData[weaponComp->m_currentWeapon].m_segmentShootEntities)[numBullet];
    GeneralCollisionComponent *genColl = m_ecsManager.getComponentManager().getGeneralCollisionComponent(bulletEntity);
    SegmentCollisionComponent *segmentColl = m_ecsManager.getComponentManager().getSegmentCollisionComponent(bulletEntity);
    assert(genColl);
    ShotConfComponent *shotComp = m_ecsManager.getComponentManager().getShotConfComponent(bulletEntity);
    assert(shotComp);
    MoveableComponent *moveImpactComp = m_ecsManager.getComponentManager().getMoveableComponent(shotComp->m_impactEntity);
    assert(moveImpactComp);
    ImpactShotComponent *impactComp = m_ecsManager.getComponentManager().getImpactShotComponent(shotComp->m_impactEntity);
    assert(impactComp);
    assert(segmentColl);
    confBullet(*impactComp, *genColl, *segmentColl, *moveImpactComp, CollisionTag_e::BULLET_PLAYER_CT, point, degreeAngle);
}

//===================================================================
void confActionShape(MapCoordComponent &mapCompAction, GeneralCollisionComponent &genCompAction,
                     const MapCoordComponent &attackerMapComp, const MoveableComponent &attackerMoveComp)
{
    mapCompAction.m_absoluteMapPositionPX = attackerMapComp.m_absoluteMapPositionPX;
    moveElementFromAngle(LEVEL_HALF_TILE_SIZE_PX, getRadiantAngle(attackerMoveComp.m_degreeOrientation),
                         mapCompAction.m_absoluteMapPositionPX);
    genCompAction.m_active = true;
}

//===================================================================
void confBullet(ImpactShotComponent &impactComp, GeneralCollisionComponent &genColl,
                SegmentCollisionComponent &segmentColl, MoveableComponent &moveImpactComp,
                CollisionTag_e collTag, const PairFloat_t &point, float degreeAngle)
{
    assert(collTag == CollisionTag_e::BULLET_ENEMY_CT || collTag == CollisionTag_e::BULLET_PLAYER_CT);
    moveImpactComp.m_degreeOrientation = degreeAngle;
    genColl.m_tagA = collTag;
    genColl.m_shape = CollisionShape_e::SEGMENT_C;
    genColl.m_active = true;
    float diff = std::rand() / ((RAND_MAX + 1u) / 9) - 4.0f;
    impactComp.m_currentVerticalPos = randFloat(-0.4f, -0.2f);
    segmentColl.m_degreeOrientation = degreeAngle + diff;
    if(segmentColl.m_degreeOrientation < EPSILON_FLOAT)
    {
        segmentColl.m_degreeOrientation += 360.0f;
    }
    segmentColl.m_points.first = point;
}

//===================================================================
void MainEngine::setUnsetPaused()
{
    m_gamePaused = !m_gamePaused;
    if(m_gamePaused)
    {
        m_ecsManager.getComponentManager().getPlayerConfComp().m_currentCursorPos = 0;
        memTimerPausedValue();
    }
    else
    {
        applyTimerPausedValue();
    }
}

//===================================================================
void MainEngine::updateTriggerWallMoveableWallDataCheckpoint(const std::pair<uint32_t, TriggerWallCheckpointData> &triggerWallCheckpointData)
{
    uint32_t shapeNum = triggerWallCheckpointData.first, wallNum = triggerWallCheckpointData.second.m_wallNum;
    assert(m_memTriggerWallMoveableWallCheckpointData.find(shapeNum) != m_memTriggerWallMoveableWallCheckpointData.end());
    assert(wallNum < m_memTriggerWallMoveableWallCheckpointData[shapeNum].first.size());
    ++m_memTriggerWallMoveableWallCheckpointData[shapeNum].first[wallNum];
}

//===================================================================
void MainEngine::clearLevel()
{
    m_audioEngine.clearSourceAndBuffer();
    m_physicalEngine.clearSystems();
    m_graphicEngine.clearSystems();
    m_memTriggerCreated.clear();
    m_ecsManager.getEngine().RmAllEntity();
    m_memWall.clear();
    clearMemSoundElements();
}

//===================================================================
void MainEngine::confSystems()
{
    m_graphicEngine.confSystems();
}

//===================================================================
void MainEngine::clearObjectToDelete()
{
    const std::vector<uint32_t> &vect = m_physicalEngine.getStaticEntitiesToDelete();
    const std::vector<uint32_t> &vectBarrelsCheckpointRem = m_physicalEngine.getBarrelEntitiesDestruct();
    const std::vector<uint32_t> &vectBarrels = m_graphicEngine.getBarrelEntitiesToDelete();
    if(vect.empty() && vectBarrels.empty() && vectBarrelsCheckpointRem.empty())
    {
        return;
    }
    for(uint32_t i = 0; i < vect.size(); ++i)
    {
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(vect[i]);
        assert(mapComp);
        m_memStaticEntitiesDeletedFromCheckpoint.insert(mapComp->m_coord);
        m_ecsManager.bRmEntity(vect[i]);
    }
    //mem destruct barrel current checkpoint
    for(uint32_t i = 0; i < vectBarrelsCheckpointRem.size(); ++i)
    {
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(vectBarrelsCheckpointRem[i]);
        assert(mapComp);
        m_memStaticEntitiesDeletedFromCheckpoint.insert(mapComp->m_coord);
    }
    //clear barrel current game entities
    for(uint32_t i = 0; i < vectBarrels.size(); ++i)
    {
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(vectBarrels[i]);
        assert(mapComp);
        m_ecsManager.bRmEntity(vectBarrels[i]);
    }
    m_physicalEngine.clearVectObjectToDelete();
    m_physicalEngine.clearVectBarrelsDestruct();
    m_graphicEngine.clearBarrelEntitiesToDelete();
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
        timerComp = m_ecsManager.getComponentManager().getTimerComponent(vectEntities[i]);
        assert(timerComp);
        time_t time = (std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) -
                       std::chrono::system_clock::to_time_t(timerComp->m_clock));
        m_vectMemPausedTimer.emplace_back(vectEntities[i], time);
    }
}

//===================================================================
void MainEngine::applyTimerPausedValue()
{
    TimerComponent *timerComp;
    for(uint32_t i = 0; i < m_vectMemPausedTimer.size(); ++i)
    {
        timerComp = m_ecsManager.getComponentManager().getTimerComponent(m_vectMemPausedTimer[i].first);
        assert(timerComp);
        timerComp->m_clock = std::chrono::system_clock::from_time_t( std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now()) - m_vectMemPausedTimer[i].second);
    }
    m_vectMemPausedTimer.clear();
}

//===================================================================
void MainEngine::loadColorEntities()
{
    uint32_t damageEntity = createColorEntity(),
            getObjectEntity = createColorEntity(),
            scratchEntity = createColorEntity(),
            transitionEntity = createColorEntity(),
            musicVolume = createColorEntity(),
            turnSensitivity = createColorEntity(),
            effectVolume = createColorEntity();
    confUnifiedColorEntity(transitionEntity, {0.0f, 0.0f, 0.0f}, true);
    confUnifiedColorEntity(damageEntity, {0.7f, 0.2f, 0.1f}, true);
    confUnifiedColorEntity(getObjectEntity, {0.1f, 0.7f, 0.5f}, true);
    confUnifiedColorEntity(scratchEntity, {0.0f, 0.0f, 0.0f}, false);
    confMenuBarMenuEntity(musicVolume, effectVolume, turnSensitivity);
    m_ecsManager.getSystemManager().searchSystemByType<ColorDisplaySystem>(static_cast<uint32_t>(Systems_e::COLOR_DISPLAY_SYSTEM))->
            loadColorEntities(damageEntity, getObjectEntity, transitionEntity, scratchEntity, musicVolume, effectVolume, turnSensitivity);
}

//===================================================================
void MainEngine::confMenuBarMenuEntity(uint32_t musicEntity, uint32_t effectEntity, uint32_t turnSensitivity)
{
    //MUSIC VOLUME
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().getPosVertexComponent(musicEntity);
    assert(posComp);
    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().getColorVertexComponent(musicEntity);
    assert(colorComp);
    float leftPos = LEFT_POS_STD_MENU_BAR, rightPos = leftPos + 0.01f + (getMusicVolume() * MAX_BAR_MENU_SIZE) / 100.0f,
    upPos = MAP_MENU_DATA.at(MenuMode_e::SOUND).first.second - 0.01f,
    downPos = upPos - (MENU_FONT_SIZE - 0.02f);
    if(!posComp->m_vertex.empty())
    {
        posComp->m_vertex.clear();
    }
    posComp->m_vertex.reserve(4);
    posComp->m_vertex.emplace_back(PairFloat_t{leftPos, upPos});
    posComp->m_vertex.emplace_back(PairFloat_t{rightPos, upPos});
    posComp->m_vertex.emplace_back(PairFloat_t{rightPos, downPos});
    posComp->m_vertex.emplace_back(PairFloat_t{leftPos, downPos});
    if(!colorComp->m_vertex.empty())
    {
        colorComp->m_vertex.clear();
    }
    colorComp->m_vertex.reserve(4);
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    //EFFECT VOLUME
    posComp = m_ecsManager.getComponentManager().getPosVertexComponent(effectEntity);
    assert(posComp);
    colorComp = m_ecsManager.getComponentManager().getColorVertexComponent(effectEntity);
    assert(colorComp);
    upPos -= MENU_FONT_SIZE;
    downPos -= MENU_FONT_SIZE;
    rightPos = leftPos + 0.01f + (getEffectsVolume() * MAX_BAR_MENU_SIZE) / 100.0f;
    posComp->m_vertex.reserve(4);
    posComp->m_vertex.emplace_back(PairFloat_t{leftPos, upPos});
    posComp->m_vertex.emplace_back(PairFloat_t{rightPos, upPos});
    posComp->m_vertex.emplace_back(PairFloat_t{rightPos, downPos});
    posComp->m_vertex.emplace_back(PairFloat_t{leftPos, downPos});
    colorComp->m_vertex.reserve(4);
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    //TURN SENSITIVITY
    posComp = m_ecsManager.getComponentManager().getPosVertexComponent(turnSensitivity);
    assert(posComp);
    colorComp = m_ecsManager.getComponentManager().getColorVertexComponent(turnSensitivity);
    assert(colorComp);
    upPos = (MAP_MENU_DATA.at(MenuMode_e::INPUT).first.second - 0.01f) -
            MENU_FONT_SIZE * static_cast<uint32_t>(InputMenuCursorPos_e::TURN_SENSITIVITY),
    downPos = upPos - MENU_FONT_SIZE;
    rightPos = 0.1f + ((getTurnSensitivity() - MIN_TURN_SENSITIVITY) * MAX_BAR_MENU_SIZE) / DIFF_TOTAL_SENSITIVITY;
    posComp->m_vertex.reserve(4);
    posComp->m_vertex.emplace_back(PairFloat_t{leftPos, upPos});
    posComp->m_vertex.emplace_back(PairFloat_t{rightPos, upPos});
    posComp->m_vertex.emplace_back(PairFloat_t{rightPos, downPos});
    posComp->m_vertex.emplace_back(PairFloat_t{leftPos, downPos});
    colorComp->m_vertex.reserve(4);
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{0.5f, 0.0f, 0.0f, 1.0f});
}


//===================================================================
void MainEngine::confUnifiedColorEntity(uint32_t entityNum, const tupleFloat_t &color, bool transparent)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().getPosVertexComponent(entityNum);
    assert(posComp);
    if(!posComp->m_vertex.empty())
    {
        posComp->m_vertex.clear();
    }
    posComp->m_vertex.reserve(4);
    posComp->m_vertex.emplace_back(PairFloat_t{-1.0f, 1.0f});
    posComp->m_vertex.emplace_back(PairFloat_t{1.0f, 1.0f});
    posComp->m_vertex.emplace_back(PairFloat_t{1.0f, -1.0f});
    posComp->m_vertex.emplace_back(PairFloat_t{-1.0f, -1.0f});
    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().getColorVertexComponent(entityNum);
    assert(colorComp);
    if(!colorComp->m_vertex.empty())
    {
        colorComp->m_vertex.clear();
    }
    colorComp->m_vertex.reserve(4);
    float alpha = transparent ? 0.4f : 1.0f;
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{std::get<0>(color), std::get<1>(color), std::get<2>(color), alpha});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{std::get<0>(color), std::get<1>(color), std::get<2>(color), alpha});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{std::get<0>(color), std::get<1>(color), std::get<2>(color), alpha});
    colorComp->m_vertex.emplace_back(TupleTetraFloat_t{std::get<0>(color), std::get<1>(color), std::get<2>(color), alpha});
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
uint32_t MainEngine::createCheckpointEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::CHECKPOINT_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::RECTANGLE_COLLISION_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createLogEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::POSITION_VERTEX_COMPONENT] = true;
    bitsetComponents[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::LOG_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createSecretEntity()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::RECTANGLE_COLLISION_COMPONENT] = true;
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
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
    loadFogEntities();
}

//===================================================================
void MainEngine::loadFogEntities()
{
    uint32_t entity = createBackgroundEntity(true);
    //GROUND
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().getPosVertexComponent(entity);
    assert(posComp);
    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().getColorVertexComponent(entity);
    assert(colorComp);
    //ceiling + dark fog middle + Ground
    posComp->m_vertex.reserve(12);
    colorComp->m_vertex.reserve(12);
    //ceiling
    posComp->m_vertex.emplace_back(-1.0f, 0.3f);
    posComp->m_vertex.emplace_back(1.0f, 0.3f);
    posComp->m_vertex.emplace_back(1.0f, 0.12f);
    posComp->m_vertex.emplace_back(-1.0f, 0.12f);

    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);

    //fog
    posComp->m_vertex.emplace_back(-1.0f, 0.12f);
    posComp->m_vertex.emplace_back(1.0f, 0.12f);
    posComp->m_vertex.emplace_back(1.0f, -0.12f);
    posComp->m_vertex.emplace_back(-1.0f, -0.12f);

    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);

    //ground
    posComp->m_vertex.emplace_back(-1.0f, -0.12f);
    posComp->m_vertex.emplace_back(1.0f, -0.12f);
    posComp->m_vertex.emplace_back(1.0f, -0.3f);
    posComp->m_vertex.emplace_back(-1.0f, -0.3f);

    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);
    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
    colorComp->m_vertex.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
    memFogColorEntity(entity);
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
void MainEngine::loadExistingLevelNumSaves(const std::array<std::optional<DataLevelWriteMenu>, 3> &existingLevelNum)
{
    m_graphicEngine.loadExistingLevelNumSaves(existingLevelNum);
}

//===================================================================
void MainEngine::loadExistingCustomLevel(const std::vector<std::string> &customLevels)
{
    m_graphicEngine.loadExistingCustomLevel(customLevels);
}

//===================================================================
void MainEngine::loadLevel(const LevelManager &levelManager)
{
    m_memWallPos.clear();
    m_physicalEngine.clearVectObjectToDelete();
    m_physicalEngine.clearVectBarrelsDestruct();
    m_graphicEngine.clearBarrelEntitiesToDelete();
    loadBackgroundEntities(levelManager.getPictureData().getGroundData(),
                           levelManager.getPictureData().getCeilingData(),
                           levelManager);
    Level::initLevelElementArray();
    if(m_memCheckpointLevelState)
    {
        if(m_memCustomLevelLoadedData)
        {
            assert(m_memCustomLevelLoadedData->m_checkpointLevelData);
            loadCheckpointSavedGame(*m_memCustomLevelLoadedData->m_checkpointLevelData, true);
        }
        else
        {
            std::optional<MemLevelLoadedData> savedData = m_refGame->loadSavedGame(m_currentSave);
            assert(savedData);
            loadCheckpointSavedGame(*savedData->m_checkpointLevelData, true);
        }
    }
    bool exit = loadStaticElementEntities(levelManager);
    loadBarrelElementEntities(levelManager);
    loadPlayerEntity(levelManager);
    loadWallEntities(levelManager.getMoveableWallData(), levelManager.getPictureData().getSpriteData());
    loadDoorEntities(levelManager);
    exit |= loadEnemiesEntities(levelManager);
    assert(exit);
    loadCheckpointsEntities(levelManager);
    loadSecretsEntities(levelManager);
    loadLogsEntities(levelManager, levelManager.getPictureData().getSpriteData());
    loadRevealedMap();
    std::string prologue = treatInfoMessageEndLine(levelManager.getLevelPrologue()),
            epilogue = treatInfoMessageEndLine(levelManager.getLevelEpilogue());
    m_graphicEngine.updatePrologueAndEpilogue(prologue, epilogue);
    //MUUUUUUUUUUUUSSSSS
    m_audioEngine.memoriseEpilogueMusicFilename(levelManager.getLevelEpilogueMusic());
    m_audioEngine.loadMusicFromFile(levelManager.getLevel().getMusicFilename());
    m_audioEngine.playMusic();
}

//===================================================================
void MainEngine::loadGameProgressCheckpoint()
{
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(m_playerEntity);
    assert(mapComp);
    MoveableComponent *moveComp = m_ecsManager.getComponentManager().getMoveableComponent(m_playerEntity);
    assert(moveComp);
    PositionVertexComponent *pos = m_ecsManager.getComponentManager().getPosVertexComponent(m_playerEntity);
    assert(pos);
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    mapComp->m_absoluteMapPositionPX = getCenteredAbsolutePosition(m_memCheckpointLevelState->m_playerPos);
    moveComp->m_degreeOrientation = getDegreeAngleFromDirection(m_memCheckpointLevelState->m_direction);
    m_memStaticEntitiesDeletedFromCheckpoint = m_currentEntitiesDelete;
    updatePlayerArrow(*moveComp, *pos);
    playerConf.m_currentCheckpoint = {m_memCheckpointLevelState->m_checkpointNum, m_memCheckpointLevelState->m_direction};
    playerConf.m_enemiesKilled = m_memCheckpointLevelState->m_ennemiesKilled;
    playerConf.m_secretsFound = m_memCheckpointLevelState->m_secretsFound;
}

//===================================================================
uint32_t MainEngine::loadWeaponsEntity(const LevelManager &levelManager)
{
    uint32_t weaponEntity = createWeaponEntity(), weaponToTreat;
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    const std::vector<WeaponINIData> &vectWeapons = levelManager.getWeaponsData();
    MemSpriteDataComponent *memSprite = m_ecsManager.getComponentManager().getMemSpriteDataComponent(weaponEntity);
    MemPositionsVertexComponents *memPosVertex = m_ecsManager.getComponentManager().getMemPositionsVertexComponent(weaponEntity);
    WeaponComponent *weaponComp = m_ecsManager.getComponentManager().getWeaponComponent(weaponEntity);
    AudioComponent *audioComp = m_ecsManager.getComponentManager().getAudioComponent(weaponEntity);
    assert(audioComp);
    assert(weaponComp);
    assert(memSprite);
    assert(memPosVertex);
    weaponComp->m_weaponsData.resize(vectWeapons.size());
    audioComp->m_soundElements.resize(vectWeapons.size());
    m_vectMemWeaponsDefault.resize(vectWeapons.size());
    std::fill(m_vectMemWeaponsDefault.begin(), m_vectMemWeaponsDefault.end(), std::pair<bool, uint32_t>{false, 0});
    for(uint32_t i = 0; i < weaponComp->m_weaponsData.size(); ++i)
    {
        weaponComp->m_weaponsData[i].m_ammunationsCount = 0;
        weaponComp->m_weaponsData[i].m_posses = false;
    }
    uint32_t totalSize = 0;
    for(uint32_t i = 0; i < vectWeapons.size(); ++i)
    {
        totalSize += vectWeapons[i].m_spritesData.size();
    }
    memSprite->m_vectSpriteData.reserve(totalSize);
    float posUp, posDown = DOWN_WEAPON_POS_Y, posLeft, posRight, diffLateral;
    memSprite->m_vectSpriteData.reserve(vectWeapons.size());
    for(uint32_t i = 0; i < vectWeapons.size(); ++i)
    {
        weaponToTreat = vectWeapons[i].m_order;
        if(vectWeapons[i].m_startingPossess)
        {
            m_vectMemWeaponsDefault[i].first = true;
            weaponComp->m_weaponsData[weaponToTreat].m_posses = true;
            weaponComp->m_currentWeapon = vectWeapons[i].m_order;
            weaponComp->m_previousWeapon = vectWeapons[i].m_order;
        }
        if(vectWeapons[i].m_startingAmmoCount)
        {
            m_vectMemWeaponsDefault[i].second = *vectWeapons[i].m_startingAmmoCount;
            weaponComp->m_weaponsData[weaponToTreat].m_ammunationsCount = *vectWeapons[i].m_startingAmmoCount;
        }
        weaponComp->m_weaponsData[weaponToTreat].m_weaponPower = vectWeapons[i].m_damage;
        weaponComp->m_weaponsData[weaponToTreat].m_animMode = vectWeapons[i].m_animMode;
        weaponComp->m_weaponsData[weaponToTreat].m_intervalLatency = vectWeapons[i].m_animationLatency / FPS_VALUE;
        weaponComp->m_weaponsData[weaponToTreat].m_visibleShotID = vectWeapons[i].m_visibleShootID;
        weaponComp->m_weaponsData[weaponToTreat].m_weaponName = vectWeapons[i].m_weaponName;
        weaponComp->m_weaponsData[weaponToTreat].m_impactID = vectWeapons[i].m_impactID;
        weaponComp->m_weaponsData[weaponToTreat].m_shotVelocity = vectWeapons[i].m_shotVelocity;
        weaponComp->m_weaponsData[weaponToTreat].m_maxAmmunations = vectWeapons[i].m_maxAmmo;
        weaponComp->m_weaponsData[weaponToTreat].m_memPosSprite = {memSprite->m_vectSpriteData.size(),
                                                                   memSprite->m_vectSpriteData.size() + vectWeapons[i].m_spritesData.size() - 1};
        weaponComp->m_weaponsData[weaponToTreat].m_lastAnimNum = memSprite->m_vectSpriteData.size() + vectWeapons[i].m_lastAnimNum;
        weaponComp->m_weaponsData[weaponToTreat].m_attackType = vectWeapons[i].m_attackType;
        weaponComp->m_weaponsData[weaponToTreat].m_simultaneousShots = vectWeapons[i].m_simultaneousShots;
        if(!vectWeapons[i].m_shotSound.empty())
        {
            audioComp->m_soundElements[weaponToTreat] = loadSound(vectWeapons[i].m_shotSound);
            m_audioEngine.memAudioMenuSound(audioComp->m_soundElements[weaponToTreat]->m_sourceALID);
        }
        if(!vectWeapons[i].m_reloadSound.empty())
        {
            weaponComp->m_reloadSoundAssociated.insert({weaponToTreat,
                                                       audioComp->m_soundElements.size()});
            audioComp->m_soundElements.push_back(loadSound(vectWeapons[i].m_reloadSound));
        }
        weaponComp->m_weaponsData[weaponToTreat].m_damageRay = vectWeapons[i].m_damageCircleRay;
        for(uint32_t j = 0; j < vectWeapons[i].m_spritesData.size(); ++j)
        {
            memSprite->m_vectSpriteData.emplace_back(&vectSprite[vectWeapons[i].m_spritesData[j].m_numSprite]);
            posUp = DOWN_WEAPON_POS_Y + vectWeapons[i].m_spritesData[j].m_GLSize.second;
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
    weaponComp->m_previewDisplayData = levelManager.getWeaponsPreviewData();
    assert(weaponComp->m_currentWeapon < vectWeapons.size());
    return weaponEntity;
}

//===================================================================
void MainEngine::loadWallEntities(const std::map<std::string, MoveableWallData> &wallData,
                                  const std::vector<SpriteData> &vectSprite)
{
    assert(!Level::getLevelCaseType().empty());
    TriggerWallMoveType_e memTriggerType;
    bool moveable;
    uint32_t shapeNum = 0;
    std::vector<uint32_t> vectMemEntities;
    bool loadFromCheckpoint = (m_memCheckpointLevelState != std::nullopt), distantTrigger;
    if(!loadFromCheckpoint)
    {
        m_memMoveableWallCheckpointData.clear();
        m_memTriggerWallMoveableWallCheckpointData.clear();
    }
    //Shape Wall Loop
    for(std::map<std::string, MoveableWallData>::const_iterator iter = wallData.begin(); iter != wallData.end(); ++iter, ++shapeNum)
    {
        vectMemEntities.clear();
        assert(!iter->second.m_sprites.empty());
        assert(iter->second.m_sprites[0] < vectSprite.size());
        moveable = !(iter->second.m_directionMove.empty());
        memTriggerType = iter->second.m_triggerType;
        distantTrigger = (memTriggerType == TriggerWallMoveType_e::BUTTON || memTriggerType == TriggerWallMoveType_e::GROUND);
        if(moveable)
        {
            if(!loadFromCheckpoint)
            {
                if(distantTrigger)
                {
                    m_memMoveableWallCheckpointData.insert({shapeNum, {0, iter->second.m_triggerBehaviourType == TriggerBehaviourType_e::REVERSABLE}});
                }
                else
                {
                    m_memTriggerWallMoveableWallCheckpointData.insert({shapeNum, {}});
                    m_memTriggerWallMoveableWallCheckpointData[shapeNum].first.resize(iter->second.m_TileGamePosition.size());
                }
            }
        }
        vectMemEntities = loadWallEntitiesWallLoop(vectSprite, *iter, moveable, shapeNum, loadFromCheckpoint);
        if(distantTrigger)
        {
            loadTriggerEntityData(iter->second, vectMemEntities, vectSprite, memTriggerType, shapeNum);
        }
    }
}

//===================================================================
std::vector<uint32_t> MainEngine::loadWallEntitiesWallLoop(const std::vector<SpriteData> &vectSprite,
                                                           const std::pair<std::string, MoveableWallData> &currentShape,
                                                           bool moveable, uint32_t shapeNum, bool loadFromCheckpoint)
{
    std::vector<uint32_t> vectMemEntities;
    const SpriteData &memSpriteData = vectSprite[currentShape.second.m_sprites[0]];
    uint32_t wallNum = 0;
    pairI_t moveableWallCorrectedPos;
    //Wall Loop
    for(std::set<PairUI_t>::const_iterator it = currentShape.second.m_TileGamePosition.begin();
        it != currentShape.second.m_TileGamePosition.end(); ++it, ++wallNum)
    {
        moveableWallCorrectedPos = {0, 0};
        if(currentShape.second.m_removeGamePosition.find(*it) != currentShape.second.m_removeGamePosition.end())
        {
            m_memWallPos.erase(*it);
            continue;
        }
        uint32_t numEntity = createWallEntity(currentShape.second.m_sprites.size() > 1, moveable);
        std::map<PairUI_t, uint32_t>::iterator itt = m_memWallPos.find(*it);
        if(itt != m_memWallPos.end())
        {
            m_ecsManager.bRmEntity(m_memWallPos[*it]);
            m_memWallPos[*it] = numEntity;
        }
        else
        {
            m_memWallPos.insert({*it, numEntity});
        }
        //if load from checkpoint
        if(loadFromCheckpoint)
        {
            if(!m_memMoveableWallCheckpointData.empty() && currentShape.second.m_triggerType != TriggerWallMoveType_e::WALL)
            {
                moveableWallCorrectedPos = getModifMoveableWallDataCheckpoint(currentShape.second.m_directionMove,
                                                                              m_memMoveableWallCheckpointData[shapeNum].first,
                                                                              currentShape.second.m_triggerBehaviourType);
            }
            else if(moveable && !m_memTriggerWallMoveableWallCheckpointData.empty() && currentShape.second.m_triggerType == TriggerWallMoveType_e::WALL)
            {
                assert(m_memTriggerWallMoveableWallCheckpointData.find(shapeNum) != m_memTriggerWallMoveableWallCheckpointData.end());
                assert(wallNum < m_memTriggerWallMoveableWallCheckpointData[shapeNum].first.size());
                moveableWallCorrectedPos = getModifMoveableWallDataCheckpoint(currentShape.second.m_directionMove,
                                                                              m_memTriggerWallMoveableWallCheckpointData[shapeNum].first[wallNum],
                                                                              currentShape.second.m_triggerBehaviourType);
            }
        }
        moveableWallCorrectedPos = {it->first + moveableWallCorrectedPos.first, it->second + moveableWallCorrectedPos.second};
        if(moveableWallCorrectedPos.first < 0)
        {
            moveableWallCorrectedPos.first = 0;
        }
        if(moveableWallCorrectedPos.second < 0)
        {
            moveableWallCorrectedPos.second = 0;
        }
        confBaseWallData(numEntity, memSpriteData, moveableWallCorrectedPos,
                         currentShape.second.m_sprites, currentShape.second.m_cyclesTime,
                         vectSprite, currentShape.second.m_triggerBehaviourType, moveable);
        if(!moveable)
        {
            continue;
        }
        vectMemEntities.emplace_back(numEntity);
        MoveableComponent *moveComp = m_ecsManager.getComponentManager().getMoveableComponent(numEntity);
        assert(moveComp);
        moveComp->m_velocity = currentShape.second.m_velocity;
        MoveableWallConfComponent *moveWallConfComp = m_ecsManager.getComponentManager().getMoveableWallConfComponent(numEntity);
        assert(moveWallConfComp);
        moveWallConfComp->setIDEntityAssociated(numEntity);
        moveWallConfComp->m_directionMove = currentShape.second.m_directionMove;
        moveWallConfComp->m_triggerType = currentShape.second.m_triggerType;
        if(moveWallConfComp->m_triggerType == TriggerWallMoveType_e::WALL &&
                !(currentShape.second.m_triggerBehaviourType == TriggerBehaviourType_e::ONCE &&
                  m_memTriggerWallMoveableWallCheckpointData[shapeNum].first[wallNum] > 0))
        {
            moveWallConfComp->m_triggerWallCheckpointData = {shapeNum, {wallNum, 0}};
            GeneralCollisionComponent *genCollComp = m_ecsManager.getComponentManager().getGeneralCollisionComponent(numEntity);
            assert(genCollComp);
            genCollComp->m_tagB = CollisionTag_e::TRIGGER_CT;
        }
        moveWallConfComp->m_triggerBehaviour = currentShape.second.m_triggerBehaviourType;
        moveWallConfComp->m_manualTrigger = (moveWallConfComp->m_triggerBehaviour == TriggerBehaviourType_e::AUTO);
        //if load a reversable wall from checkpoint reverse direction
        if(currentShape.second.m_triggerBehaviourType == TriggerBehaviourType_e::REVERSABLE &&
                m_memMoveableWallCheckpointData[shapeNum].first % 2 == 1)
        {
            reverseDirection(*moveWallConfComp);
        }
    }
    return vectMemEntities;
}

//===================================================================
void MainEngine::confBaseWallData(uint32_t wallEntity, const SpriteData &memSpriteData,
                                  const PairUI_t& coordLevel,
                                  const std::vector<uint16_t> &numWallSprites,
                                  const std::vector<uint32_t> &timeMultiSpriteCase,
                                  const std::vector<SpriteData> &vectSprite, TriggerBehaviourType_e triggerType, bool moveable)
{
    MemSpriteDataComponent *memSpriteComp;
    SpriteTextureComponent *spriteComp;
    TimerComponent *timerComp;
    confBaseComponent(wallEntity, memSpriteData, coordLevel,
                      CollisionShape_e::RECTANGLE_C, CollisionTag_e::WALL_CT);
    if(!timeMultiSpriteCase.empty())
    {
        WallMultiSpriteConf *multiSpriteConf = m_ecsManager.getComponentManager().getWallMultiSpriteConfComponent(wallEntity);
        assert(multiSpriteConf);
        multiSpriteConf->m_cyclesTime = timeMultiSpriteCase;
    }
    spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(wallEntity);
    assert(spriteComp);
    LevelCaseType_e type = moveable ? LevelCaseType_e::WALL_MOVE_LC : LevelCaseType_e::WALL_LC;
    std::optional<ElementRaycast> element  = Level::getElementCase(coordLevel);
    if(moveable && triggerType != TriggerBehaviourType_e::AUTO &&
            (!element || element->m_typeStd != LevelCaseType_e::WALL_LC))
    {
        Level::memStaticMoveWallEntity(coordLevel, wallEntity);
    }
    Level::addElementCase(spriteComp, coordLevel, type, wallEntity);
    if(numWallSprites.size() == 1)
    {
        return;
    }
    memSpriteComp = m_ecsManager.getComponentManager().getMemSpriteDataComponent(wallEntity);
    assert(memSpriteComp);
    uint32_t vectSize = numWallSprites.size();
    memSpriteComp->m_vectSpriteData.reserve(static_cast<uint32_t>(WallSpriteType_e::TOTAL_SPRITE));
    for(uint32_t j = 0; j < vectSize; ++j)
    {
        memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[numWallSprites[j]]);
    }
    timerComp = m_ecsManager.getComponentManager().getTimerComponent(wallEntity);
    assert(timerComp);
    timerComp->m_cycleCountA = 0;
}

//===================================================================
void MainEngine::loadDoorEntities(const LevelManager &levelManager)
{
    const std::map<std::string, DoorData> &doorData = levelManager.getDoorData();
    MemSpriteDataComponent *memSpriteComp;
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    std::map<std::string, DoorData>::const_iterator it = doorData.begin();
    SoundElement currentSoundElement = loadSound(levelManager.getDoorOpeningSoundFile());
    for(; it != doorData.end(); ++it)
    {
        const SpriteData &memSpriteData = levelManager.getPictureData().getSpriteData()[it->second.m_numSprite];
        for(uint32_t j = 0; j < it->second.m_TileGamePosition.size(); ++j)
        {
            uint32_t numEntity = createDoorEntity();
            confBaseComponent(numEntity, memSpriteData, it->second.m_TileGamePosition[j],
                              CollisionShape_e::RECTANGLE_C, CollisionTag_e::DOOR_CT);
            MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(numEntity);
            assert(mapComp);
            RectangleCollisionComponent *rectComp = m_ecsManager.getComponentManager().getRectangleCollisionComponent(numEntity);
            assert(rectComp);
            DoorComponent *doorComp = m_ecsManager.getComponentManager().getDoorComponent(numEntity);
            assert(doorComp);
            AudioComponent *audioComp = m_ecsManager.getComponentManager().getAudioComponent(numEntity);
            assert(audioComp);
            TimerComponent *timerComp = m_ecsManager.getComponentManager().getTimerComponent(numEntity);
            assert(audioComp);
            timerComp->m_cycleCountA = 0;
            audioComp->m_soundElements.push_back(currentSoundElement);
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
            memSpriteComp = m_ecsManager.getComponentManager().getMemSpriteDataComponent(numEntity);
            SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(numEntity);
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
bool MainEngine::loadEnemiesEntities(const LevelManager &levelManager)
{
    const std::map<std::string, EnemyData> &enemiesData = levelManager.getEnemiesData();
    float collisionRay;
    bool exit = false;
    std::array<SoundElement, 4> currentSoundElements;
    bool loadFromCheckpoint = (!m_memEnemiesStateFromCheckpoint.empty());
    m_currentLevelEnemiesNumber = 0;
    m_currentLevelEnemiesKilled = 0;
    for(std::map<std::string, EnemyData>::const_iterator it = enemiesData.begin(); it != enemiesData.end(); ++it)
    {
        currentSoundElements[0] = loadSound(it->second.m_normalBehaviourSoundFile);
        currentSoundElements[1] = loadSound(it->second.m_detectBehaviourSoundFile);
        currentSoundElements[2] = loadSound(it->second.m_attackSoundFile);
        currentSoundElements[3] = loadSound(it->second.m_deathSoundFile);
        collisionRay = it->second.m_inGameSpriteSize.first * LEVEL_TWO_THIRD_TILE_SIZE_PX;
        const SpriteData &memSpriteData = levelManager.getPictureData().
                getSpriteData()[it->second.m_staticFrontSprites[0]];
        for(uint32_t j = 0; j < it->second.m_TileGamePosition.size(); ++j)
        {
            exit |= createEnemy(levelManager, memSpriteData, it->second, collisionRay, loadFromCheckpoint, j, currentSoundElements);
        }
    }
    return exit;
}

//===================================================================
bool MainEngine::createEnemy(const LevelManager &levelManager, const SpriteData &memSpriteData, const EnemyData &enemyData,
                             float collisionRay, bool loadFromCheckpoint, uint32_t index, const std::array<SoundElement, 4> &soundElements)
{
    bool exit = false;
    uint32_t numEntity = createEnemyEntity();
    confBaseComponent(numEntity, memSpriteData, enemyData.m_TileGamePosition[index],
                      CollisionShape_e::CIRCLE_C, CollisionTag_e::ENEMY_CT);
    EnemyConfComponent *enemyComp = m_ecsManager.getComponentManager().getEnemyConfComponent(numEntity);
    FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(numEntity);
    assert(enemyComp);
    assert(fpsStaticComp);
    if(enemyData.m_endLevelPos && (*enemyData.m_endLevelPos) == enemyData.m_TileGamePosition[index])
    {
        enemyComp->m_endLevel = true;
        exit = true;
    }
    fpsStaticComp->m_inGameSpriteSize = enemyData.m_inGameSpriteSize;
    fpsStaticComp->m_levelElementType = LevelStaticElementType_e::GROUND;
    CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().getCircleCollComponent(numEntity);
    assert(circleComp);
    circleComp->m_ray = collisionRay;
    enemyComp->m_life = enemyData.m_life;
    enemyComp->m_visibleShot = !(enemyData.m_visibleShootID.empty());
    enemyComp->m_countTillLastAttack = 0;
    enemyComp->m_meleeOnly = enemyData.m_meleeOnly;
    enemyComp->m_frozenOnAttack = enemyData.m_frozenOnAttack;
    if(enemyData.m_meleeDamage)
    {
        enemyComp->m_meleeAttackDamage = *enemyData.m_meleeDamage;
    }
    if(!enemyData.m_dropedObjectID.empty())
    {
        enemyComp->m_dropedObjectEntity = createEnemyDropObject(levelManager, enemyData, index, loadFromCheckpoint, m_currentLevelEnemiesNumber);
    }
    if(enemyComp->m_visibleShot)
    {
        if(!loadFromCheckpoint || !m_memEnemiesStateFromCheckpoint[m_currentLevelEnemiesNumber].m_dead)
        {
            enemyComp->m_visibleAmmo.resize(4);
            confAmmoEntities(enemyComp->m_visibleAmmo, CollisionTag_e::BULLET_ENEMY_CT,
                             enemyComp->m_visibleShot, enemyData.m_attackPower,
                             enemyData.m_shotVelocity, enemyData.m_damageZone);
        }
    }
    else
    {
        loadNonVisibleEnemyAmmoStuff(loadFromCheckpoint, m_currentLevelEnemiesNumber, enemyData, levelManager, enemyComp);
    }
    loadEnemySprites(levelManager.getPictureData().getSpriteData(),
                     enemyData, numEntity, enemyComp, levelManager.getVisibleShootDisplayData());
    MoveableComponent *moveComp = m_ecsManager.getComponentManager().getMoveableComponent(numEntity);
    assert(moveComp);
    moveComp->m_velocity = enemyData.m_velocity;
    moveComp->m_currentDegreeMoveDirection = 0.0f;
    moveComp->m_degreeOrientation = 0.0f;
    AudioComponent *audiocomponent = m_ecsManager.getComponentManager().getAudioComponent(numEntity);
    assert(audiocomponent);
    audiocomponent->m_soundElements.reserve(4);
    audiocomponent->m_soundElements.emplace_back(soundElements[0]);
    audiocomponent->m_soundElements.emplace_back(soundElements[1]);
    audiocomponent->m_soundElements.emplace_back(soundElements[2]);
    audiocomponent->m_soundElements.emplace_back(soundElements[3]);
    audiocomponent->m_maxDistance /= 5.0f;
    TimerComponent *timerComponent = m_ecsManager.getComponentManager().getTimerComponent(numEntity);
    assert(timerComponent);
    timerComponent->m_cycleCountA = 0;
    memCheckpointEnemiesData(loadFromCheckpoint, numEntity, m_currentLevelEnemiesNumber);
    ++m_currentLevelEnemiesNumber;
    return exit;
}

//===================================================================
pairI_t getModifMoveableWallDataCheckpoint(const std::vector<std::pair<Direction_e, uint32_t>> &vectDir,
                                           uint32_t timesActionned, TriggerBehaviourType_e triggerBehaviour)
{
    if(timesActionned == 0 || (triggerBehaviour == TriggerBehaviourType_e::REVERSABLE && timesActionned % 2 == 0))
    {
        return {0, 0};
    }
    else
    {
        pairI_t posModif = {0, 0};
        for(uint32_t i = 0; i < vectDir.size(); ++i)
        {
            switch(vectDir[i].first)
            {
            case Direction_e::EAST:
                posModif.first += vectDir[i].second;
                break;
            case Direction_e::WEST:
                posModif.first -= vectDir[i].second;
                break;
            case Direction_e::NORTH:
                posModif.second -= vectDir[i].second;
                break;
            case Direction_e::SOUTH:
                posModif.second += vectDir[i].second;
                break;
            }
        }
        if(triggerBehaviour != TriggerBehaviourType_e::REVERSABLE)
        {
            posModif.first *= timesActionned;
            posModif.second *= timesActionned;
        }
        return posModif;
    }
}

//===================================================================
void MainEngine::loadNonVisibleEnemyAmmoStuff(bool loadFromCheckpoint, uint32_t currentEnemy,
                                              const EnemyData &enemyData, const LevelManager &levelManager,
                                              EnemyConfComponent *enemyComp)
{
    if(loadFromCheckpoint && m_memEnemiesStateFromCheckpoint[currentEnemy].m_dead)
    {
        return;
    }
    enemyComp->m_stdAmmo.resize(MAX_SHOTS);
    confAmmoEntities(enemyComp->m_stdAmmo, CollisionTag_e::BULLET_ENEMY_CT, enemyComp->m_visibleShot,
                     enemyData.m_attackPower);
    const MapImpactData_t &map = levelManager.getImpactDisplayData();
    MapImpactData_t::const_iterator itt = map.find(enemyData.m_impactID);
    assert(itt != map.end());
    for(uint32_t j = 0; j < enemyComp->m_stdAmmo.size(); ++j)
    {
        ShotConfComponent *shotComp = m_ecsManager.getComponentManager().getShotConfComponent(enemyComp->m_stdAmmo[j]);
        assert(shotComp);
        shotComp->m_impactEntity = confShotImpactEntity(levelManager.getPictureSpriteData(), itt->second);
    }
}

//===================================================================
void MainEngine::memCheckpointEnemiesData(bool loadFromCheckpoint, uint32_t enemyEntity, uint32_t cmpt)
{
    if(loadFromCheckpoint)
    {
        m_memEnemiesStateFromCheckpoint[cmpt].m_entityNum = enemyEntity;
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(enemyEntity);
        assert(mapComp);
        EnemyConfComponent *enemyComp = m_ecsManager.getComponentManager().getEnemyConfComponent(enemyEntity);
        assert(enemyComp);
        mapComp->m_absoluteMapPositionPX = m_memEnemiesStateFromCheckpoint[cmpt].m_enemyPos;
        enemyComp->m_life = m_memEnemiesStateFromCheckpoint[cmpt].m_life;
        if(m_memEnemiesStateFromCheckpoint[cmpt].m_dead)
        {
            enemyComp->m_displayMode = EnemyDisplayMode_e::DEAD;
            enemyComp->m_behaviourMode = EnemyBehaviourMode_e::DEAD;
            enemyComp->m_currentSprite = enemyComp->m_mapSpriteAssociate.find(EnemySpriteType_e::DYING)->second.second;
        }
    }
    else
    {
        m_memEnemiesStateFromCheckpoint.push_back({enemyEntity, 0, false, false, {}});
    }
}

//===================================================================
void MainEngine::loadCheckpointsEntities(const LevelManager &levelManager)
{
    const std::vector<std::pair<PairUI_t, Direction_e>> &container = levelManager.getCheckpointsData();
    uint32_t entityNum;
    for(uint32_t i = 0; i < container.size(); ++i)
    {
        if(m_currentEntitiesDelete.find(container[i].first) != m_currentEntitiesDelete.end())
        {
            continue;
        }
        entityNum = createCheckpointEntity();
        initStdCollisionCase(entityNum, container[i].first, CollisionTag_e::CHECKPOINT_CT);
        CheckpointComponent *checkComponent = m_ecsManager.getComponentManager().getCheckpointComponent(entityNum);
        assert(checkComponent);
        checkComponent->m_checkpointNumber = i + 1;
        checkComponent->m_direction = container[i].second;
    }
}


//===================================================================
void MainEngine::loadSecretsEntities(const LevelManager &levelManager)
{
    const std::vector<PairUI_t> &container = levelManager.getSecretsData();
    m_currentLevelSecretsNumber = container.size();
    uint32_t entityNum;
    for(uint32_t i = 0; i < container.size(); ++i)
    {
        if(m_currentEntitiesDelete.find(container[i]) != m_currentEntitiesDelete.end())
        {
            continue;
        }
        entityNum = createCheckpointEntity();
        initStdCollisionCase(entityNum, container[i], CollisionTag_e::SECRET_CT);
    }
}

//===================================================================
void MainEngine::loadLogsEntities(const LevelManager &levelManager, const std::vector<SpriteData> &vectSprite)
{
    const std::vector<LogLevelData> &container = levelManager.getLogsData();
    const std::map<std::string, LogStdData> &stdLogData = levelManager.getStdLogData();
    uint32_t entityNum;
    std::map<std::string, LogStdData>::const_iterator it;
    for(uint32_t i = 0; i < container.size(); ++i)
    {
        entityNum = createLogEntity();
        it = stdLogData.find(container[i].m_id);
        confBaseComponent(entityNum, vectSprite[it->second.m_spriteNum], container[i].m_pos,
                CollisionShape_e::CIRCLE_C, CollisionTag_e::LOG_CT);
        FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(entityNum);
        assert(fpsStaticComp);
        assert(it != stdLogData.end());
        fpsStaticComp->m_inGameSpriteSize = it->second.m_fpsSize;
        fpsStaticComp->m_levelElementType = LevelStaticElementType_e::GROUND;
        CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().getCircleCollComponent(entityNum);
        assert(circleComp);
        circleComp->m_ray = 10.0f;
        LogComponent *logComp = m_ecsManager.getComponentManager().getLogComponent(entityNum);
        assert(logComp);
        logComp->m_message = treatInfoMessageEndLine(container[i].m_message);
        SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(entityNum);
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(entityNum);
        assert(spriteComp);
        assert(mapComp);
        Level::addElementCase(spriteComp, mapComp->m_coord, LevelCaseType_e::EMPTY_LC, entityNum);
    }
}

//===================================================================
void MainEngine::loadRevealedMap()
{
    m_graphicEngine.getMapSystem().clearRevealedMap();
    for(uint32_t i = 0; i < m_revealedMapData.size(); ++i)
    {
        std::optional<ElementRaycast> element = Level::getElementCase(m_revealedMapData[i]);
        assert(element);
        m_graphicEngine.getMapSystem().addDiscoveredEntity(element->m_numEntity, m_revealedMapData[i]);
    }
}

//===================================================================
void MainEngine::initStdCollisionCase(uint32_t entityNum, const PairUI_t &mapPos, CollisionTag_e tag)
{
    MapCoordComponent *mapComponent = m_ecsManager.getComponentManager().getMapCoordComponent(entityNum);
    assert(mapComponent);
    mapComponent->m_coord = mapPos;
    mapComponent->m_absoluteMapPositionPX = {mapPos.first * LEVEL_TILE_SIZE_PX, mapPos.second * LEVEL_TILE_SIZE_PX};
    GeneralCollisionComponent *genCompComponent = m_ecsManager.getComponentManager().getGeneralCollisionComponent(entityNum);
    assert(genCompComponent);
    genCompComponent->m_shape = CollisionShape_e::RECTANGLE_C;
    genCompComponent->m_tagA = tag;
    RectangleCollisionComponent *rectComponent = m_ecsManager.getComponentManager().getRectangleCollisionComponent(entityNum);
    assert(rectComponent);
    rectComponent->m_size = {LEVEL_TILE_SIZE_PX, LEVEL_TILE_SIZE_PX};
}

//===================================================================
void MainEngine::loadTriggerEntityData(const MoveableWallData &moveWallData,
                                       const std::vector<uint32_t> &vectPosition,
                                       const std::vector<SpriteData> &vectSprite,
                                       TriggerWallMoveType_e type, uint32_t shapeNum)
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
    uint32_t numEntity;
    bool newTrigger = (it == m_memTriggerCreated.end());
    if(it == m_memTriggerCreated.end())
    {
        numEntity = createTriggerEntity(button);
    }
    else
    {
        numEntity = it->second;
    }
    //if trigger does not exist
    if(newTrigger)
    {
        m_memTriggerCreated.insert({pos, numEntity});
        GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().getGeneralCollisionComponent(numEntity);
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(numEntity);
        CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().getCircleCollComponent(numEntity);
        assert(circleComp);
        assert(mapComp);
        assert(genComp);
        if(button)
        {
            SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(numEntity);
            FPSVisibleStaticElementComponent *fpsComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(numEntity);
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
    }
    //if shape wall already actionned and once behaviour
    if(moveWallData.m_triggerBehaviourType == TriggerBehaviourType_e::ONCE && m_memMoveableWallCheckpointData[shapeNum].first > 0)
    {
        return;
    }
    //add new entity num to trigger
    TriggerComponent *triggerComp = m_ecsManager.getComponentManager().getTriggerComponent(numEntity);
    assert(triggerComp);
    triggerComp->m_mapElementEntities.insert({shapeNum, vectPosition});
}

//===================================================================
uint32_t MainEngine::createEnemyDropObject(const LevelManager &levelManager, const EnemyData &enemyData,
                                           uint32_t iterationNum, bool loadFromCheckpoint, uint32_t cmpt)
{
    std::map<std::string, StaticLevelElementData>::const_iterator itt = levelManager.getObjectData().find(enemyData.m_dropedObjectID);
    assert(itt != levelManager.getObjectData().end());
    std::optional<uint32_t> objectEntity = createStaticElementEntity(LevelStaticElementType_e::OBJECT, itt->second,
                                                      levelManager.getPictureSpriteData(), iterationNum, true);
    assert(objectEntity);
    GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().getGeneralCollisionComponent(*objectEntity);
    assert(genComp);
    genComp->m_active = false;
    if(loadFromCheckpoint && m_memEnemiesStateFromCheckpoint[cmpt].m_dead &&
            !m_memEnemiesStateFromCheckpoint[cmpt].m_objectPickedUp)
    {
        genComp->m_active = true;
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(*objectEntity);
        assert(mapComp);
        mapComp->m_absoluteMapPositionPX = m_memEnemiesStateFromCheckpoint[cmpt].m_enemyPos;
    }
    return *objectEntity;
}

//===================================================================
void MainEngine::createPlayerAmmoEntities(PlayerConfComponent *playerConf, CollisionTag_e collTag)
{
    WeaponComponent *weaponComp = m_ecsManager.getComponentManager().getWeaponComponent(
        playerConf->m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)]);
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
        ShotConfComponent *shotConfComp = m_ecsManager.getComponentManager().getShotConfComponent(ammoEntities[j]);
        assert(shotConfComp);
        shotConfComp->m_damage = damage;
        if(damageRay)
        {
            shotConfComp->m_damageCircleRayData = createDamageZoneEntity(damage, CollisionTag_e::EXPLOSION_CT, LEVEL_TILE_SIZE_PX);
        }
        if(visibleShot)
        {
            MoveableComponent *moveComp = m_ecsManager.getComponentManager().getMoveableComponent(ammoEntities[j]);
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
    GeneralCollisionComponent *genColl = m_ecsManager.getComponentManager().getGeneralCollisionComponent(ammoNum);
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
void MainEngine::setMenuEntries(PlayerConfComponent &playerComp, std::optional<uint32_t> cursorPos)
{
    if(playerComp.m_menuMode == MenuMode_e::CONFIRM_RESTART_FROM_LAST_CHECKPOINT && !m_memCheckpointLevelState)
    {
        playerComp.m_menuMode = MenuMode_e::BASE;
        return;
    }
    m_ecsManager.getComponentManager().getPlayerConfComp().m_currentCursorPos = cursorPos ? *cursorPos : 0;
    //TITLE MENU
    WriteComponent *writeComp = m_ecsManager.getComponentManager().getWriteComponent(
        playerComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::TITLE_MENU)]);
    assert(writeComp);
    m_graphicEngine.fillTitleMenuWrite(*writeComp, playerComp.m_menuMode, playerComp.m_previousMenuMode);
    //MENU ENTRIES
    WriteComponent *writeConf = m_ecsManager.getComponentManager().getWriteComponent(
            playerComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MENU_ENTRIES)]);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = MAP_MENU_DATA.at(playerComp.m_menuMode).first;
    if(writeConf->m_vectMessage.empty())
    {
        writeConf->addTextLine({{}, ""});
    }
    //SELECTED MENU ENTRY
    writeComp = m_ecsManager.getComponentManager().getWriteComponent(
        playerComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MENU_SELECTED_LINE)]);
    assert(writeComp);
    m_graphicEngine.fillMenuWrite(*writeConf, playerComp.m_menuMode, playerComp.m_currentCursorPos,
                                  {&playerComp, m_currentLevelSecretsNumber, m_currentLevelEnemiesNumber});
    if(playerComp.m_menuMode == MenuMode_e::LEVEL_PROLOGUE ||
            playerComp.m_menuMode == MenuMode_e::LEVEL_EPILOGUE ||
            playerComp.m_menuMode == MenuMode_e::TRANSITION_LEVEL)
    {
        return;
    }
    m_graphicEngine.confMenuSelectedLine(playerComp, *writeComp, *writeConf);
    if(playerComp.m_menuMode == MenuMode_e::INPUT)
    {
        updateConfirmLoadingMenuInfo(playerComp);
    }
    else if(playerComp.m_menuMode == MenuMode_e::CONFIRM_QUIT_INPUT_FORM ||
            playerComp.m_menuMode == MenuMode_e::CONFIRM_LOADING_GAME_FORM ||
            playerComp.m_menuMode == MenuMode_e::CONFIRM_RESTART_LEVEL ||
            playerComp.m_menuMode == MenuMode_e::CONFIRM_RESTART_FROM_LAST_CHECKPOINT)
    {
        updateConfirmLoadingMenuInfo(playerComp);
        playerComp.m_currentCursorPos = 0;
    }
    else if(playerComp.m_menuMode != MenuMode_e::NEW_KEY && playerComp.m_menuMode != MenuMode_e::TITLE
            && playerComp.m_menuMode != MenuMode_e::BASE)
    {
        playerComp.m_currentCursorPos = 0;
    }
    if(playerComp.m_menuMode == MenuMode_e::DISPLAY || playerComp.m_menuMode == MenuMode_e::SOUND ||
            playerComp.m_menuMode == MenuMode_e::INPUT || playerComp.m_menuMode == MenuMode_e::LOAD_GAME ||
            playerComp.m_menuMode == MenuMode_e::LOAD_CUSTOM_LEVEL || playerComp.m_menuMode == MenuMode_e::NEW_GAME ||
            playerComp.m_menuMode == MenuMode_e::TITLE || playerComp.m_menuMode == MenuMode_e::BASE)
    {
        writeConf->m_vectMessage[0].first = writeConf->m_upLeftPositionGL.first;
    }
    else
    {
        writeConf->m_vectMessage[0].first = {};
    }
    m_graphicEngine.updateMenuCursorPosition(playerComp);
}

//===================================================================
void MainEngine::updateConfirmLoadingMenuInfo(PlayerConfComponent &playerComp)
{
    WriteComponent *writeComp = m_ecsManager.getComponentManager().getWriteComponent(
        playerComp.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MENU_INFO_WRITE)]);
    assert(writeComp);
    writeComp->clear();
    writeComp->m_fontSpriteData.reserve(4);
    writeComp->m_vectMessage.reserve(4);
    if(playerComp.m_menuMode == MenuMode_e::INPUT)
    {
        writeComp->m_upLeftPositionGL = {-0.6f, -0.7f};
        writeComp->addTextLine({writeComp->m_upLeftPositionGL.first, ""});
        writeComp->m_vectMessage.back().second = playerComp.m_keyboardInputMenuMode ? "Keyboard\\Switch Gamepad : G Or RL" :
                                                                 "Gamepad\\Switch Keyboard : G Or RL";
    }
    else if(playerComp.m_menuMode == MenuMode_e::CONFIRM_QUIT_INPUT_FORM)
    {
        writeComp->m_upLeftPositionGL = {-0.6f, 0.3f};
        writeComp->addTextLine({{}, "Do You Want To Save Changes?"});
    }
    else if(playerComp.m_menuMode == MenuMode_e::CONFIRM_LOADING_GAME_FORM ||
            playerComp.m_menuMode == MenuMode_e::CONFIRM_RESTART_LEVEL ||
            playerComp.m_menuMode == MenuMode_e::CONFIRM_RESTART_FROM_LAST_CHECKPOINT ||
            playerComp.m_menuMode == MenuMode_e::CONFIRM_QUIT_GAME)
    {
        if(!playerComp.m_firstMenu)
        {
            writeComp->m_upLeftPositionGL = {-0.8f, 0.5f};
            writeComp->addTextLine({{}, "All Your Progress Until Last Save"});
            writeComp->addTextLine({{}, "Will Be Lost"});
        }
        else
        {
            writeComp->m_upLeftPositionGL = {-0.8f, 0.5f};
        }
        if(playerComp.m_menuMode == MenuMode_e::CONFIRM_LOADING_GAME_FORM)
        {
            if(playerComp.m_previousMenuMode == MenuMode_e::NEW_GAME && checkSavedGameExists(playerComp.m_currentCursorPos + 1))
            {
                writeComp->addTextLine({{}, "Previous File Will Be Erased"});
            }
            if(!writeComp->m_vectMessage.empty())
            {
                writeComp->addTextLine({{}, "Continue Anyway?"});
            }
            //TITLE MENU CASE
            else
            {
                writeComp->m_upLeftPositionGL = {-0.3f, 0.3f};
                if(playerComp.m_previousMenuMode == MenuMode_e::NEW_GAME)
                {
                    writeComp->addTextLine({{}, "Begin New Game?"});
                }
                else if(playerComp.m_previousMenuMode == MenuMode_e::LOAD_GAME)
                {
                    writeComp->addTextLine({{}, "Load Game?"});
                }
                else if(playerComp.m_previousMenuMode == MenuMode_e::LOAD_CUSTOM_LEVEL)
                {
                    writeComp->addTextLine({{}, "Load Custom Game?"});
                }
            }
        }
        else if(playerComp.m_menuMode == MenuMode_e::CONFIRM_QUIT_GAME)
        {
            writeComp->addTextLine({{}, "Do You Really Want To Quit The Game?"});
        }
    }
    m_graphicEngine.confWriteComponent(*writeComp);
}

//===================================================================
void MainEngine::updateWriteComp(WriteComponent &writeComp)
{
    m_graphicEngine.confWriteComponent(writeComp);
}

//===================================================================
void MainEngine::updateStringWriteEntitiesInputMenu(bool keyboardInputMenuMode, bool defaultInput)
{
    m_graphicEngine.updateStringWriteEntitiesInputMenu(keyboardInputMenuMode, defaultInput);
}

//===================================================================
void MainEngine::confGlobalSettings(const SettingsData &settingsData)
{
    //AUDIO
    if(settingsData.m_musicVolume > 100)
    {
        m_audioEngine.updateMusicVolume(100);
        m_graphicEngine.updateMusicVolumeBar(100);
    }
    else
    {
        if(settingsData.m_musicVolume)
        {
            m_audioEngine.updateMusicVolume(*settingsData.m_musicVolume);
            m_graphicEngine.updateMusicVolumeBar(*settingsData.m_musicVolume);
        }
    }
    if(settingsData.m_effectsVolume > 100)
    {
        m_audioEngine.updateEffectsVolume(100, false);
        m_graphicEngine.updateEffectsVolumeBar(100);
    }
    else
    {
        if(settingsData.m_musicVolume)
        {
            m_audioEngine.updateEffectsVolume(*settingsData.m_effectsVolume, false);
            m_graphicEngine.updateEffectsVolumeBar(*settingsData.m_effectsVolume);
        }
    }
    //DISPLAY
    if(settingsData.m_fullscreen && *settingsData.m_fullscreen)
    {
        m_graphicEngine.toogleFullScreen();
    }
    if(settingsData.m_resolutionWidth && settingsData.m_resolutionHeight)
    {
        m_graphicEngine.setSizeResolution({*settingsData.m_resolutionWidth, *settingsData.m_resolutionHeight});
    }
    //INPUT
    //KEYBOARD
    if(settingsData.m_arrayKeyboard)
    {
        m_physicalEngine.setKeyboardKey(*settingsData.m_arrayKeyboard);
    }
    //GAMEPAD
    if(settingsData.m_arrayGamepad)
    {
        m_physicalEngine.setGamepadKey(*settingsData.m_arrayGamepad);
    }
    if(settingsData.m_turnSensitivity)
    {
        updateTurnSensitivity(*settingsData.m_turnSensitivity);
    }
}

//===================================================================
void MainEngine::validDisplayMenu()
{
    m_graphicEngine.validDisplayMenu();
    m_refGame->saveDisplaySettings(m_graphicEngine.getResolutions()[m_graphicEngine.getCurrentResolutionNum()].first,
            m_graphicEngine.fullscreenMode());
}

//===================================================================
void MainEngine::reinitPlayerGear()
{
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    WeaponComponent *weaponComp = m_ecsManager.getComponentManager().getWeaponComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MENU_ENTRIES)]);
    //if first launch return
    if(!weaponComp)
    {
        return;
    }
    playerConf.m_card.clear();
    playerConf.m_checkpointReached = {};
    playerConf.m_currentCheckpoint = {};
    playerConf.m_enemiesKilled = {};
    playerConf.m_life = 100;
    playerConf.m_secretsFound = {};
    for(uint32_t i = 0; i < weaponComp->m_weaponsData.size(); ++i)
    {
        weaponComp->m_weaponsData[i].m_posses = m_vectMemWeaponsDefault[i].first;
        weaponComp->m_weaponsData[i].m_ammunationsCount = m_vectMemWeaponsDefault[i].second;
    }
}

//===================================================================
void MainEngine::setInfoDataWrite(std::string_view message)
{
    m_ecsManager.getComponentManager().getPlayerConfComp().m_infoWriteData = {true, message.data()};
}

//===================================================================
void MainEngine::playTriggerSound()
{
    AudioComponent *audioComp = m_ecsManager.getComponentManager().getAudioComponent(m_playerEntity);
    assert(audioComp);
    audioComp->m_soundElements[2]->m_toPlay = true;
    m_audioEngine.getSoundSystem()->execSystem();
}

//===================================================================
void MainEngine::confMenuSelectedLine()
{
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    WriteComponent *writeMenuComp = m_ecsManager.getComponentManager().getWriteComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MENU_ENTRIES)]);
    assert(writeMenuComp);
    WriteComponent *writeMenuSelectedComp = m_ecsManager.getComponentManager().getWriteComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MENU_SELECTED_LINE)]);
    assert(writeMenuSelectedComp);
    m_graphicEngine.confMenuSelectedLine(playerConf, *writeMenuSelectedComp, *writeMenuComp);
}

//===================================================================
void MainEngine::setPlayerDeparture(const PairUI_t &pos, Direction_e dir)
{
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(m_playerEntity);
    MoveableComponent *moveComp = m_ecsManager.getComponentManager().getMoveableComponent(m_playerEntity);
    assert(mapComp);
    assert(moveComp);
    mapComp->m_absoluteMapPositionPX = getCenteredAbsolutePosition(pos);
    moveComp->m_degreeOrientation = getDegreeAngleFromDirection(dir);
}

//===================================================================
void MainEngine::saveAudioSettings()
{
    m_refGame->saveAudioSettings(getMusicVolume(), getEffectsVolume());
}

//===================================================================
void MainEngine::saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                                   const std::map<ControlKey_e, MouseKeyboardInputState> &keyboardArray)
{
    m_refGame->saveInputSettings(gamepadArray, keyboardArray);
}

//===================================================================
void MainEngine::saveTurnSensitivitySettings()
{
    m_refGame->saveTurnSensitivitySettings(m_physicalEngine.getTurnSensitivity());
}

//===================================================================
bool MainEngine::loadSavedGame(uint32_t saveNum, LevelState_e levelMode)
{
    m_memCustomLevelLoadedData = 0;
    m_currentLevelState = levelMode;
    m_currentSave = saveNum;
    if(m_currentLevelState == LevelState_e::NEW_GAME)
    {
        m_levelToLoad = {1, false};
        return true;
    }
    std::optional<MemLevelLoadedData> savedData = m_refGame->loadSavedGame(saveNum);
    if(!savedData)
    {
        return false;
    }
    m_memPlayerConfBeginLevel = *savedData->m_playerConfBeginLevel;
    if(savedData->m_checkpointLevelData)
    {
        m_ecsManager.getComponentManager().getPlayerConfComp().m_currentCheckpoint = {savedData->m_checkpointLevelData->m_checkpointNum, savedData->m_checkpointLevelData->m_direction};
    }
    assert(!m_memPlayerConfBeginLevel.m_ammunationsCount.empty());
    if(savedData->m_playerConfCheckpoint)
    {
        m_memPlayerConfCheckpoint = *savedData->m_playerConfCheckpoint;
    }
    if(m_currentLevelState == LevelState_e::RESTART_LEVEL || m_currentLevelState == LevelState_e::RESTART_FROM_CHECKPOINT)
    {
        m_levelToLoad = {m_currentLevel, false};
    }
    else if(m_currentLevelState == LevelState_e::LOAD_GAME)
    {
        m_levelToLoad = {savedData->m_levelNum, false};
    }
    m_playerMemGear = true;
    if(savedData->m_checkpointLevelData)
    {
        loadCheckpointSavedGame(*savedData->m_checkpointLevelData);
    }
    if(m_memCustomLevelLoadedData)
    {
        loadRevealedMap();
    }
    return true;
}

//===================================================================
bool MainEngine::loadCustomLevelGame(LevelState_e levelMode)
{
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    m_currentLevelState = levelMode;
    m_levelToLoad = {playerConf.m_levelToLoad, true};
    m_currentLevel = playerConf.m_levelToLoad;
    if(!m_memCustomLevelLoadedData)
    {
        m_memCustomLevelLoadedData = std::make_unique<MemCustomLevelLoadedData>();
    }
    return true;
}

//===================================================================
void MainEngine::loadCheckpointSavedGame(const MemCheckpointElementsState &checkpointData, bool loadFromSameLevel)
{
    m_memCheckpointLevelState = {m_levelToLoad->first, checkpointData.m_checkpointNum, checkpointData.m_secretsNumber,
                                 checkpointData.m_enemiesKilled, checkpointData.m_direction,
                                 checkpointData.m_checkpointPos};
    if(!loadFromSameLevel)
    {
        m_currentEntitiesDelete = checkpointData.m_staticElementDeleted;
    }
    m_memEnemiesStateFromCheckpoint = checkpointData.m_enemiesData;
    m_memMoveableWallCheckpointData = checkpointData.m_moveableWallData;
    m_memTriggerWallMoveableWallCheckpointData = checkpointData.m_triggerWallMoveableWallData;
    m_revealedMapData = checkpointData.m_revealedMapData;
    m_memCheckpointData = {checkpointData.m_checkpointNum, checkpointData.m_secretsNumber,
                           checkpointData.m_enemiesKilled, checkpointData.m_checkpointPos,
                           checkpointData.m_direction, m_memEnemiesStateFromCheckpoint,
                           m_memMoveableWallCheckpointData, m_memTriggerWallMoveableWallCheckpointData,
                           m_memStaticEntitiesDeletedFromCheckpoint, checkpointData.m_revealedMapData, checkpointData.m_card};
    m_ecsManager.getComponentManager().getPlayerConfComp().m_card = m_memCheckpointData->m_card;
}

//===================================================================
bool MainEngine::checkSavedGameExists(uint32_t saveNum)const
{
    return m_refGame->checkSavedGameExists(saveNum);
}

//===================================================================
void MainEngine::clearCheckpointData()
{
    m_memStaticEntitiesDeletedFromCheckpoint.clear();
    m_currentEntitiesDelete.clear();
    m_memMoveableWallCheckpointData.clear();
    m_memTriggerWallMoveableWallCheckpointData.clear();
    m_memCheckpointLevelState = std::nullopt;
    m_memEnemiesStateFromCheckpoint.clear();
    m_revealedMapData.clear();
    if(m_memCustomLevelLoadedData)
    {
        m_memCustomLevelLoadedData->m_checkpointLevelData = std::nullopt;
    }
    m_memCheckpointData.reset();
}

//===================================================================
bool MainEngine::isLoadFromLevelBegin(LevelState_e levelState)const
{
    if(levelState == LevelState_e::RESTART_FROM_CHECKPOINT ||
            (m_memCheckpointLevelState && (levelState == LevelState_e::GAME_OVER || levelState == LevelState_e::LOAD_GAME)))
    {
        return false;
    }
    return true;
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
                (*weaponConf->m_weaponsData[i].m_visibleShootEntities)[j] = createAmmoEntity(CollisionTag_e::BULLET_PLAYER_CT, true);
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
            for(uint32_t j = 0; j < weaponConf->m_weaponsData[i].m_segmentShootEntities->size(); ++j)
            {
                ShotConfComponent *shotComp = m_ecsManager.getComponentManager().getShotConfComponent(
                    (*weaponConf->m_weaponsData[i].m_segmentShootEntities)[j]);
                assert(shotComp);
                shotComp->m_impactEntity = confShotImpactEntity(vectSpriteData, it->second);
            }
        }
    }
}

//===================================================================
uint32_t MainEngine::confShotImpactEntity(const std::vector<SpriteData> &vectSpriteData,
                                          const PairImpactData_t &shootDisplayData)
{
    uint32_t impactEntity = createShotImpactEntity();
    FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(impactEntity);
    GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().getGeneralCollisionComponent(impactEntity);
    CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().getCircleCollComponent(impactEntity);
    assert(circleComp);
    assert(genComp);
    assert(fpsStaticComp);
    circleComp->m_ray = 2.0f;
    fpsStaticComp->m_inGameSpriteSize = shootDisplayData.first[0].m_GLSize;
    fpsStaticComp->m_levelElementType = LevelStaticElementType_e::IMPACT;
    genComp->m_active = false;
    genComp->m_tagA = CollisionTag_e::IMPACT_CT;
    genComp->m_tagB = CollisionTag_e::IMPACT_CT;
    genComp->m_shape = CollisionShape_e::CIRCLE_C;
    loadShotImpactSprite(vectSpriteData, shootDisplayData, impactEntity);
    return impactEntity;
}

//===================================================================
void MainEngine::loadEnemySprites(const std::vector<SpriteData> &vectSprite, const EnemyData &enemiesData, uint32_t numEntity,
                                  EnemyConfComponent *enemyComp, const MapVisibleShotData_t &visibleShot)
{
    MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().getMemSpriteDataComponent(numEntity);
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
                               const std::vector<uint16_t> &enemyMemArray,
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
        SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(visibleAmmo[k]);
        MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().getMemSpriteDataComponent(visibleAmmo[k]);
        FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(visibleAmmo[k]);
        MemFPSGLSizeComponent *memFPSGLSizeComp = m_ecsManager.getComponentManager().getMemFPSGLSizeComponent(visibleAmmo[k]);
        AudioComponent *audioComp = m_ecsManager.getComponentManager().getAudioComponent(visibleAmmo[k]);
        ShotConfComponent *shotComp = m_ecsManager.getComponentManager().getShotConfComponent(visibleAmmo[k]);
        assert(shotComp);
        assert(audioComp);
        assert(memFPSGLSizeComp);
        assert(fpsStaticComp);
        assert(spriteComp);
        assert(memSpriteComp);
        MapVisibleShotData_t::const_iterator it = visibleShot.find(visibleShootID);
        assert(it != visibleShot.end());
        if(!m_memSoundElements.m_visibleShots)
        {
            m_memSoundElements.m_visibleShots = std::map<std::string, SoundElement>();
        }
        if(m_memSoundElements.m_visibleShots->find(it->second.first) == m_memSoundElements.m_visibleShots->end())
        {
            m_memSoundElements.m_visibleShots->insert({it->second.first, loadSound(it->second.first)});
        }
        audioComp->m_soundElements.push_back(m_memSoundElements.m_visibleShots->at(it->second.first));
        memSpriteComp->m_vectSpriteData.reserve(it->second.second.size());
        memFPSGLSizeComp->m_memGLSizeData.reserve(it->second.second.size());
        for(uint32_t l = 0; l < it->second.second.size(); ++l)
        {
            memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[it->second.second[l].m_numSprite]);
            memFPSGLSizeComp->m_memGLSizeData.emplace_back(it->second.second[l].m_GLSize);
        }
        fpsStaticComp->m_inGameSpriteSize = it->second.second[0].m_GLSize;
        fpsStaticComp->m_levelElementType = LevelStaticElementType_e::GROUND;
        spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[0];
        float maxWidth = memFPSGLSizeComp->m_memGLSizeData[0].first;
        for(uint32_t i = 1; i < memFPSGLSizeComp->m_memGLSizeData.size(); ++i)
        {
            if(maxWidth < memFPSGLSizeComp->m_memGLSizeData[i].first)
            {
                maxWidth = memFPSGLSizeComp->m_memGLSizeData[i].first;
            }
        }
        shotComp->m_ejectExplosionRay = maxWidth * LEVEL_HALF_TILE_SIZE_PX;
    }
}

//===================================================================
void MainEngine::confVisibleAmmo(uint32_t ammoEntity)
{
    PairFloat_t pairSpriteSize = {0.2f, 0.3f};
    float collisionRay = pairSpriteSize.first * LEVEL_HALF_TILE_SIZE_PX;
    CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().getCircleCollComponent(ammoEntity);
    FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(ammoEntity);
    MoveableComponent *moveComp = m_ecsManager.getComponentManager().getMoveableComponent(ammoEntity);
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
    bitsetComponents[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComponents[Components_e::TIMER_COMPONENT] = true;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
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
    bitsetComponents[Components_e::AUDIO_COMPONENT] = true;
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
        bitsetComponents[Components_e::WALL_MULTI_SPRITE_CONF] = true;
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
    bitsetComponents[Components_e::AUDIO_COMPONENT] = true;
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
    bitsetComponents[Components_e::AUDIO_COMPONENT] = true;
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
    bitsetComponents[Components_e::AUDIO_COMPONENT] = true;
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
    bitsetComponents[Components_e::MOVEABLE_COMPONENT] = true;
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
    bitsetComponents[Components_e::AUDIO_COMPONENT] = true;
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
    bitsetComponents[Components_e::AUDIO_COMPONENT] = true;
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
                                   const std::optional<PairUI_t> &coordLevel, CollisionShape_e collisionShape,
                                   CollisionTag_e tag)
{
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(entityNum);
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(entityNum);
    assert(spriteComp);
    assert(mapComp);
    spriteComp->m_spriteData = &memSpriteData;
    if(coordLevel)
    {
        mapComp->m_coord = *coordLevel;
        if(tag == CollisionTag_e::WALL_CT || tag == CollisionTag_e::DOOR_CT)
        {
            mapComp->m_absoluteMapPositionPX = getAbsolutePosition(*coordLevel);
        }
        else
        {
            mapComp->m_absoluteMapPositionPX = getCenteredAbsolutePosition(*coordLevel);
        }
    }
    GeneralCollisionComponent *tagComp = m_ecsManager.getComponentManager().getGeneralCollisionComponent(entityNum);
    assert(tagComp);
    tagComp->m_shape = collisionShape;
    if(collisionShape == CollisionShape_e::RECTANGLE_C)
    {
        RectangleCollisionComponent *rectComp = m_ecsManager.getComponentManager().getRectangleCollisionComponent(entityNum);
        assert(rectComp);
        rectComp->m_size = {LEVEL_TILE_SIZE_PX, LEVEL_TILE_SIZE_PX};
    }
    tagComp->m_tagA = tag;
}

//===================================================================
void MainEngine::confStaticComponent(uint32_t entityNum, const PairFloat_t& elementSize,
                                     LevelStaticElementType_e elementType)
{
    FPSVisibleStaticElementComponent *staticComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(entityNum);
    assert(staticComp);
    staticComp->m_inGameSpriteSize = elementSize;
    staticComp->m_levelElementType = elementType;
}

//===================================================================
void MainEngine::loadPlayerEntity(const LevelManager &levelManager)
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
    bitsetComponents[Components_e::AUDIO_COMPONENT] = true;
    uint32_t entityNum = m_ecsManager.addEntity(bitsetComponents);
    confPlayerEntity(levelManager, entityNum, levelManager.getLevel(),
                     loadWeaponsEntity(levelManager), loadDisplayTeleportEntity(levelManager));
    //notify player entity number
    m_graphicEngine.getMapSystem().confPlayerComp(entityNum);
    m_physicalEngine.memPlayerEntity(entityNum);
    m_audioEngine.memPlayerEntity(entityNum);
}

//===================================================================
void MainEngine::confPlayerEntity(const LevelManager &levelManager,
                                  uint32_t entityNum, const Level &level,
                                  uint32_t numWeaponEntity, uint32_t numDisplayTeleportEntity)
{
    m_playerEntity = entityNum;
    const std::vector<SpriteData> &vectSpriteData =
            levelManager.getPictureData().getSpriteData();
    PositionVertexComponent *pos = m_ecsManager.getComponentManager().getPosVertexComponent(entityNum);
    MapCoordComponent *map = m_ecsManager.getComponentManager().getMapCoordComponent(entityNum);
    MoveableComponent *move = m_ecsManager.getComponentManager().getMoveableComponent(entityNum);
    ColorVertexComponent *color = m_ecsManager.getComponentManager().getColorVertexComponent(entityNum);
    CircleCollisionComponent *circleColl = m_ecsManager.getComponentManager().getCircleCollComponent(entityNum);
    GeneralCollisionComponent *tagColl = m_ecsManager.getComponentManager().getGeneralCollisionComponent(entityNum);
    PlayerConfComponent *playerConf = m_ecsManager.getComponentManager().getPlayerConfComponent(entityNum);
    assert(playerConf);
    playerConf->m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)] = numWeaponEntity;
    playerConf->m_levelToLoad = m_currentLevel;
    playerConf->setIDEntityAssociated(entityNum);
    playerConf->m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::DISPLAY_TELEPORT)] = numDisplayTeleportEntity;
    AudioComponent *audioComp = m_ecsManager.getComponentManager().getAudioComponent(entityNum);
    assert(audioComp);
    audioComp->m_soundElements.reserve(3);
    audioComp->m_soundElements.emplace_back(loadSound(levelManager.getPickObjectSoundFile()));
    audioComp->m_soundElements.emplace_back(loadSound(levelManager.getPlayerDeathSoundFile()));
    audioComp->m_soundElements.emplace_back(loadSound(levelManager.getTriggerSoundFile()));
    assert(pos);
    assert(map);
    assert(move);
    assert(color);
    assert(circleColl);
    assert(tagColl);
    WeaponComponent *weaponConf = m_ecsManager.getComponentManager().getWeaponComponent(
        playerConf->m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)]);
    assert(weaponConf);
    createPlayerAmmoEntities(playerConf, CollisionTag_e::BULLET_PLAYER_CT);
    createPlayerVisibleShotEntity(weaponConf);
    confPlayerVisibleShotsSprite(vectSpriteData, levelManager.getVisibleShootDisplayData(), weaponConf);
    createPlayerImpactEntities(vectSpriteData, weaponConf, levelManager.getImpactDisplayData());
    map->m_coord = level.getPlayerDeparture();
    Direction_e playerDir = level.getPlayerDepartureDirection();
    move->m_degreeOrientation = getDegreeAngleFromDirection(playerDir);
    move->m_velocity = 2.5f;
    map->m_absoluteMapPositionPX = getCenteredAbsolutePosition(map->m_coord);
    color->m_vertex.reserve(3);
    color->m_vertex.emplace_back(TupleTetraFloat_t{0.9f, 0.00f, 0.00f, 1.0f});
    color->m_vertex.emplace_back(TupleTetraFloat_t{0.9f, 0.00f, 0.00f, 1.0f});
    color->m_vertex.emplace_back(TupleTetraFloat_t{0.9f, 0.00f, 0.00f, 1.0f});
    circleColl->m_ray = PLAYER_RAY;
    updatePlayerArrow(*move, *pos);
    tagColl->m_tagA = CollisionTag_e::PLAYER_CT;
    tagColl->m_shape = CollisionShape_e::CIRCLE_C;
    //set standard weapon sprite
    StaticDisplaySystem *staticDisplay = m_ecsManager.getSystemManager().
            searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM));
    assert(staticDisplay);
    staticDisplay->setWeaponSprite(numWeaponEntity, weaponConf->m_weaponsData[weaponConf->m_currentWeapon].m_memPosSprite.first);
    confWriteEntities();
    confMenuEntities();
    confLifeAmmoPannelEntities();
    confWeaponsPreviewEntities();
    confActionEntity();
    confMapDetectShapeEntity(map->m_absoluteMapPositionPX);
    for(uint32_t i = 0; i < weaponConf->m_weaponsData.size(); ++i)
    {
        if(weaponConf->m_weaponsData[i].m_attackType == AttackType_e::MELEE)
        {
            playerConf->m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::HIT_MELEE)] = createDamageZoneEntity(weaponConf->m_weaponsData[i].m_weaponPower,
                                                                   CollisionTag_e::HIT_PLAYER_CT, 10.0f, levelManager.getHitSoundFile());
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
    GeneralCollisionComponent *genCollComp = m_ecsManager.getComponentManager().getGeneralCollisionComponent(entityNum);
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(entityNum);
    CircleCollisionComponent *circleColl = m_ecsManager.getComponentManager().getCircleCollComponent(entityNum);
    assert(genCollComp);
    assert(mapComp);
    assert(circleColl);
    genCollComp->m_active = false;
    genCollComp->m_shape = CollisionShape_e::CIRCLE_C;
    genCollComp->m_tagA = CollisionTag_e::PLAYER_ACTION_CT;
    circleColl->m_ray = 15.0f;    
    m_ecsManager.getComponentManager().getPlayerConfComp().m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::ACTION)] = entityNum;
}

//===================================================================
void MainEngine::confMapDetectShapeEntity(const PairFloat_t &playerPos)
{
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::RECTANGLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MAP_DETECT_SHAPE)] = m_ecsManager.addEntity(bitsetComponents);
    RectangleCollisionComponent *rectColl = m_ecsManager.getComponentManager().getRectangleCollisionComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MAP_DETECT_SHAPE)]);
    MapCoordComponent *mapComp= m_ecsManager.getComponentManager().getMapCoordComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MAP_DETECT_SHAPE)]);
    GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().getGeneralCollisionComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MAP_DETECT_SHAPE)]);
    assert(rectColl);
    assert(mapComp);
    assert(genComp);
    mapComp->m_absoluteMapPositionPX = {playerPos.first - DETECT_RECT_SHAPE_HALF_SIZE,
                                        playerPos.second - DETECT_RECT_SHAPE_HALF_SIZE};
    rectColl->m_size = {DETECT_RECT_SHAPE_SIZE, DETECT_RECT_SHAPE_SIZE};
    genComp->m_shape = CollisionShape_e::RECTANGLE_C;
    genComp->m_tagA = CollisionTag_e::DETECT_MAP_CT;
}

//===================================================================
uint32_t MainEngine::createMeleeAttackEntity(bool sound)
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComponents;
    bitsetComponents[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComponents[Components_e::CIRCLE_COLLISION_COMPONENT] = true;
    bitsetComponents[Components_e::SHOT_CONF_COMPONENT] = true;
    if(sound)
    {
        bitsetComponents[Components_e::AUDIO_COMPONENT] = true;
    }
    return m_ecsManager.addEntity(bitsetComponents);
}

//===================================================================
uint32_t MainEngine::createDamageZoneEntity(uint32_t damage, CollisionTag_e tag,
                                            float ray, const std::string &soundFile)
{
    uint32_t entityNum = createMeleeAttackEntity(!soundFile.empty());
    GeneralCollisionComponent *genCollComp = m_ecsManager.getComponentManager().getGeneralCollisionComponent(entityNum);
    CircleCollisionComponent *circleColl = m_ecsManager.getComponentManager().getCircleCollComponent(entityNum);
    ShotConfComponent *shotComp = m_ecsManager.getComponentManager().getShotConfComponent(entityNum);
    assert(shotComp);
    assert(genCollComp);
    assert(circleColl);
    if(!soundFile.empty())
    {
        AudioComponent *audioComp = m_ecsManager.getComponentManager().getAudioComponent(entityNum);
        assert(audioComp);
        if(!m_memSoundElements.m_damageZone)
        {
            m_memSoundElements.m_damageZone = loadSound(soundFile);
        }
        audioComp->m_soundElements.push_back(m_memSoundElements.m_damageZone);
    }
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
    MemSpriteDataComponent *memComp = m_ecsManager.getComponentManager().getMemSpriteDataComponent(impactEntity);
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(impactEntity);
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
    uint32_t numAmmoWrite = createWriteEntity(), numInfoWrite = createWriteEntity(), numLifeWrite = createWriteEntity(),
            numMenuWrite = createWriteEntity(), numTitleMenuWrite = createWriteEntity(),
            numInputModeMenuWrite = createWriteEntity(), numMenuSelectedLineWrite = createWriteEntity();
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    WeaponComponent *weaponConf = m_ecsManager.getComponentManager().getWeaponComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::WEAPON)]);
    assert(weaponConf);
    //INFO
    WriteComponent *writeConf = m_ecsManager.getComponentManager().getWriteComponent(numInfoWrite);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.3f, 0.7f};
    writeConf->addTextLine({{}, ""});
    writeConf->m_fontSize = STD_FONT_SIZE;
    //AMMO
    writeConf = m_ecsManager.getComponentManager().getWriteComponent(numAmmoWrite);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.8f, -0.9f};
    writeConf->m_fontSize = STD_FONT_SIZE;
    writeConf->addTextLine({writeConf->m_upLeftPositionGL.first, ""});
    m_graphicEngine.updateAmmoCount(*writeConf, *weaponConf);
    //LIFE
    writeConf = m_ecsManager.getComponentManager().getWriteComponent(numLifeWrite);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.8f, -0.8f};
    writeConf->m_fontSize = STD_FONT_SIZE;
    writeConf->addTextLine({writeConf->m_upLeftPositionGL.first, ""});
    m_graphicEngine.updatePlayerLife(*writeConf, playerConf);
    //MENU
    writeConf = m_ecsManager.getComponentManager().getWriteComponent(numMenuWrite);
    assert(writeConf);
    writeConf->m_fontSize = MENU_FONT_SIZE;
    //TITLE MENU
    writeConf = m_ecsManager.getComponentManager().getWriteComponent(numTitleMenuWrite);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.3f, 0.9f};
    writeConf->addTextLine({{}, ""});
    writeConf->m_fontSize = MENU_FONT_SIZE;
    //INPUT MENU MODE
    writeConf = m_ecsManager.getComponentManager().getWriteComponent(numInputModeMenuWrite);
    assert(writeConf);
    writeConf->m_upLeftPositionGL = {-0.6f, -0.7f};
    writeConf->m_fontSize = MENU_FONT_SIZE;
    confWriteEntitiesDisplayMenu();
    confWriteEntitiesInputMenu();
    playerConf.m_menuMode = MenuMode_e::BASE;
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MENU_ENTRIES)] = numMenuWrite;
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::TITLE_MENU)] = numTitleMenuWrite;
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MENU_INFO_WRITE)] = numInputModeMenuWrite;
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::MENU_SELECTED_LINE)] = numMenuSelectedLineWrite;
    WriteComponent *writeComp = m_ecsManager.getComponentManager().getWriteComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::TITLE_MENU)]);
    assert(writeComp);
    writeConf->addTextLine({-0.3f, ""});
    WriteComponent *writeCompTitle = m_ecsManager.getComponentManager().getWriteComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::TITLE_MENU)]);
    assert(writeCompTitle);
    writeCompTitle->m_fontSpriteData.emplace_back(VectSpriteDataRef_t{});

    //MENU SELECTED LINE
    writeComp = m_ecsManager.getComponentManager().getWriteComponent(numMenuSelectedLineWrite);
    assert(writeComp);
    writeComp->m_fontSpriteData.emplace_back(VectSpriteDataRef_t{});
    writeComp->m_fontSize = MENU_FONT_SIZE;
    writeComp->m_fontType = Font_e::SELECTED;

    setMenuEntries(playerConf);
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::AMMO_WRITE)] = numAmmoWrite;
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::LIFE_WRITE)] = numLifeWrite;
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::NUM_INFO_WRITE)] = numInfoWrite;
}

//===================================================================
void MainEngine::confWriteEntitiesDisplayMenu()
{
    uint32_t numMenuResolutionWrite = createWriteEntity(), numMenuFullscreenWrite = createWriteEntity();
    //Resolution
    WriteComponent *writeConfA = m_ecsManager.getComponentManager().getWriteComponent(numMenuResolutionWrite);
    assert(writeConfA);
    writeConfA->m_upLeftPositionGL.first = MAP_MENU_DATA.at(MenuMode_e::DISPLAY).first.first + 1.0f;
    writeConfA->m_upLeftPositionGL.second = MAP_MENU_DATA.at(MenuMode_e::DISPLAY).first.second;
    writeConfA->m_fontSize = MENU_FONT_SIZE;
    if(writeConfA->m_vectMessage.empty())
    {
        writeConfA->addTextLine({writeConfA->m_upLeftPositionGL.first, ""});
    }
    //OOOOK default resolution
    writeConfA->m_vectMessage[0].second = m_graphicEngine.getResolutions()[0].second;
    m_graphicEngine.confWriteComponent(*writeConfA);
    //Fullscreen
    WriteComponent *writeConfB = m_ecsManager.getComponentManager().getWriteComponent(numMenuFullscreenWrite);
    assert(writeConfB);
    writeConfB->m_upLeftPositionGL = {writeConfA->m_upLeftPositionGL.first, writeConfA->m_upLeftPositionGL.second - MENU_FONT_SIZE};
    writeConfB->m_fontSize = MENU_FONT_SIZE;
    if(writeConfB->m_vectMessage.empty())
    {
        writeConfB->addTextLine({writeConfB->m_upLeftPositionGL.first, ""});
    }
    writeConfB->m_vectMessage[0].second = "";
    m_graphicEngine.confWriteComponent(*writeConfB);
    m_ecsManager.getSystemManager().searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM))->
            memDisplayMenuEntities(numMenuResolutionWrite, numMenuFullscreenWrite);
}

//===================================================================
void MainEngine::confWriteEntitiesInputMenu()
{
    ArrayControlKey_t memKeyboardEntities, memGamepadEntities;
    WriteComponent *writeConf;
    PairFloat_t currentUpLeftPos = {MAP_MENU_DATA.at(MenuMode_e::INPUT).first.first + 1.0f, MAP_MENU_DATA.at(MenuMode_e::INPUT).first.second};
    for(uint32_t i = 0; i < memKeyboardEntities.size(); ++i)
    {
        //KEYBOARD
        memKeyboardEntities[i] = createWriteEntity();
        writeConf = m_ecsManager.getComponentManager().getWriteComponent(memKeyboardEntities[i]);
        assert(writeConf);
        writeConf->m_upLeftPositionGL = currentUpLeftPos;
        writeConf->m_fontSize = MENU_FONT_SIZE;
        //GAMEPAD
        memGamepadEntities[i] = createWriteEntity();
        writeConf = m_ecsManager.getComponentManager().getWriteComponent(memGamepadEntities[i]);
        assert(writeConf);
        writeConf->m_upLeftPositionGL = currentUpLeftPos;
        writeConf->m_fontSize = MENU_FONT_SIZE;
        currentUpLeftPos.second -= MENU_FONT_SIZE;
    }
    m_ecsManager.getSystemManager().searchSystemByType<StaticDisplaySystem>(static_cast<uint32_t>(Systems_e::STATIC_DISPLAY_SYSTEM))->
            memInputMenuEntities(memKeyboardEntities, memGamepadEntities);
    updateStringWriteEntitiesInputMenu(true);
}

//===================================================================
void MainEngine::confMenuEntities()
{
    assert(m_memBackgroundGenericMenu);
    m_ecsManager.getComponentManager().getPlayerConfComp().m_menuMode = MenuMode_e::BASE;
    confMenuEntity(PlayerEntities_e::MENU_TITLE_BACKGROUND);
    confMenuEntity(PlayerEntities_e::MENU_GENERIC_BACKGROUND);
    confMenuEntity(PlayerEntities_e::MENU_LEFT_BACKGROUND);
    confMenuEntity(PlayerEntities_e::MENU_RIGHT_LEFT_BACKGROUND);
}

//===================================================================
void MainEngine::confMenuEntity(PlayerEntities_e entityType)
{
    uint32_t backgroundEntity = createSimpleSpriteEntity();
    PositionVertexComponent *posVertex = m_ecsManager.getComponentManager().getPosVertexComponent(backgroundEntity);
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(backgroundEntity);
    assert(posVertex);
    assert(spriteComp);
    m_ecsManager.getComponentManager().getPlayerConfComp().m_vectEntities[static_cast<uint32_t>(entityType)] = backgroundEntity;
    if(entityType == PlayerEntities_e::MENU_GENERIC_BACKGROUND)
    {
        spriteComp->m_spriteData = m_memBackgroundGenericMenu;
    }
    else if(entityType == PlayerEntities_e::MENU_TITLE_BACKGROUND)
    {
        spriteComp->m_spriteData = m_memBackgroundTitleMenu;
    }
    else if(entityType == PlayerEntities_e::MENU_LEFT_BACKGROUND)
    {
        spriteComp->m_spriteData = m_memBackgroundLeftMenu;
    }
    else if(entityType == PlayerEntities_e::MENU_RIGHT_LEFT_BACKGROUND)
    {
        spriteComp->m_spriteData = m_memBackgroundRightLeftMenu;
    }
    else
    {
        assert(false);
    }
    posVertex->m_vertex.resize(4);
    posVertex->m_vertex[0] = {-1.0f, 1.0f};
    posVertex->m_vertex[1] = {1.0f, 1.0f};
    posVertex->m_vertex[2] = {1.0f, -1.0f};
    posVertex->m_vertex[3] = {-1.0f, -1.0f};
}

//===================================================================
void MainEngine::confLifeAmmoPannelEntities()
{
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    //PANNEL
    uint32_t lifeAmmoPannelEntity = createSimpleSpriteEntity();
    PositionVertexComponent *posCursor = m_ecsManager.getComponentManager().getPosVertexComponent(lifeAmmoPannelEntity);
    SpriteTextureComponent *spriteCursor = m_ecsManager.getComponentManager().getSpriteTextureComponent(lifeAmmoPannelEntity);
    assert(posCursor);
    assert(spriteCursor);
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::LIFE_AMMO_PANNEL)] = lifeAmmoPannelEntity;
    spriteCursor->m_spriteData = m_memPannel;
    posCursor->m_vertex.reserve(4);
    float up = -0.78f, down = -0.97f, left = -0.97f, right = -0.625f;
    posCursor->m_vertex.insert(posCursor->m_vertex.end(), {{left, up}, {right, up}, {right, down},{left, down}});
    //LIFE
    uint32_t lifeIconEntity = createSimpleSpriteEntity();
    posCursor = m_ecsManager.getComponentManager().getPosVertexComponent(lifeIconEntity);
    spriteCursor = m_ecsManager.getComponentManager().getSpriteTextureComponent(lifeIconEntity);
    assert(posCursor);
    assert(spriteCursor);
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::LIFE_ICON)] = lifeIconEntity;
    spriteCursor->m_spriteData = m_memLifeIcon;
    posCursor->m_vertex.reserve(4);
    up = -0.8f, down = -0.87f, left = -0.95f, right = -0.9f;
    posCursor->m_vertex.insert(posCursor->m_vertex.end(), {{left, up}, {right, up}, {right, down},{left, down}});
    //AMMO
    uint32_t ammoIconEntity = createSimpleSpriteEntity();
    posCursor = m_ecsManager.getComponentManager().getPosVertexComponent(ammoIconEntity);
    spriteCursor = m_ecsManager.getComponentManager().getSpriteTextureComponent(ammoIconEntity);
    assert(posCursor);
    assert(spriteCursor);
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::AMMO_ICON)] = ammoIconEntity;
    spriteCursor->m_spriteData = m_memAmmoIcon;
    posCursor->m_vertex.reserve(4);
    up = -0.9f, down = -0.95f, left = -0.95f, right = -0.9f;
    posCursor->m_vertex.insert(posCursor->m_vertex.end(), {{left, up}, {right, up}, {right, down},{left, down}});

}

//===================================================================
void MainEngine::confWeaponsPreviewEntities()
{
    PlayerConfComponent &playerConf = m_ecsManager.getComponentManager().getPlayerConfComp();
    playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::CURSOR_WEAPON_PREVIEW)] = createSimpleSpriteEntity();
    SpriteTextureComponent *spriteCursor = m_ecsManager.getComponentManager().getSpriteTextureComponent(
        playerConf.m_vectEntities[static_cast<uint32_t>(PlayerEntities_e::CURSOR_WEAPON_PREVIEW)]);
    assert(spriteCursor);
    spriteCursor->m_spriteData = m_memPannel;
    for(uint32_t i = 0; i < playerConf.m_vectPossessedWeaponsPreviewEntities.size(); ++i)
    {
        playerConf.m_vectPossessedWeaponsPreviewEntities[i] = createSimpleSpriteEntity();
        spriteCursor = m_ecsManager.getComponentManager().getSpriteTextureComponent(playerConf.m_vectPossessedWeaponsPreviewEntities[i]);
        assert(spriteCursor);
        switch(i)
        {
        case 0:
            spriteCursor->m_spriteData = m_memPreviewFistIcon;
            break;
        case 1:
            spriteCursor->m_spriteData = m_memPreviewGunIcon;
            break;
        case 2:
            spriteCursor->m_spriteData = m_memPreviewShotgunIcon;
            break;
        case 3:
            spriteCursor->m_spriteData = m_memPreviewPlasmaRifleIcon;
            break;
        case 4:
            spriteCursor->m_spriteData = m_memPreviewBazookaIcon;
            break;
        case 5:
            spriteCursor->m_spriteData = m_memPreviewMachineGunIcon;
            break;
        default:
            assert(false);
            break;
        }
    }
}

//===================================================================
bool MainEngine::loadStaticElementEntities(const LevelManager &levelManager)
{
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    loadStaticElementGroup(vectSprite, levelManager.getGroundData(), LevelStaticElementType_e::GROUND);
    loadStaticElementGroup(vectSprite, levelManager.getCeilingData(), LevelStaticElementType_e::CEILING);
    loadStaticElementGroup(vectSprite, levelManager.getObjectData(), LevelStaticElementType_e::OBJECT);
    loadStaticElementGroup(vectSprite, levelManager.getTeleportData(), LevelStaticElementType_e::TELEPORT, levelManager.getTeleportSoundFile());
    return loadExitElement(levelManager, levelManager.getExitElementData());
}

//===================================================================
void MainEngine::loadStaticSpriteEntities(const LevelManager &levelManager)
{
    uint16_t pannelSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getPannelSpriteName()),
            lifeIconSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getLifeIconSpriteName()),
            ammoIconSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getAmmoIconSpriteName()),
            fistIconSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getFistIconSpriteName()),
            gunIconSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getGunIconSpriteName()),
            shotgunIconSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getShotgunIconSpriteName()),
            plasmaRifleIconSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getPlasmaRifleIconSpriteName()),
            machineGunIconSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getMachineGunIconSpriteName()),
            GenericBackgroundSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getGenericMenuSpriteName()),
            TitleBackgroundSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getTitleMenuSpriteName()),
            leftBackgroundSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getLeftMenuSpriteName()),
            rightLeftBackgroundSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getRightLeftMenuSpriteName()),
            bazookaIconSpriteId = *levelManager.getPictureData().getIdentifier(levelManager.getBazookaIconSpriteName());

    m_memBackgroundGenericMenu = &levelManager.getPictureData().getSpriteData()[GenericBackgroundSpriteId];
    m_memBackgroundTitleMenu = &levelManager.getPictureData().getSpriteData()[TitleBackgroundSpriteId];
    m_memBackgroundLeftMenu = &levelManager.getPictureData().getSpriteData()[leftBackgroundSpriteId];
    m_memBackgroundRightLeftMenu = &levelManager.getPictureData().getSpriteData()[rightLeftBackgroundSpriteId];
    m_memPannel = &levelManager.getPictureData().getSpriteData()[pannelSpriteId];
    m_memLifeIcon = &levelManager.getPictureData().getSpriteData()[lifeIconSpriteId];
    m_memAmmoIcon = &levelManager.getPictureData().getSpriteData()[ammoIconSpriteId];

    m_memPreviewFistIcon = &levelManager.getPictureData().getSpriteData()[fistIconSpriteId];
    m_memPreviewGunIcon = &levelManager.getPictureData().getSpriteData()[gunIconSpriteId];
    m_memPreviewShotgunIcon = &levelManager.getPictureData().getSpriteData()[shotgunIconSpriteId];
    m_memPreviewPlasmaRifleIcon = &levelManager.getPictureData().getSpriteData()[plasmaRifleIconSpriteId];
    m_memPreviewMachineGunIcon = &levelManager.getPictureData().getSpriteData()[machineGunIconSpriteId];
    m_memPreviewBazookaIcon = &levelManager.getPictureData().getSpriteData()[bazookaIconSpriteId];
}

//===================================================================
void MainEngine::loadBarrelElementEntities(const LevelManager &levelManager)
{
    const BarrelData &barrelData = levelManager.getBarrelData();
    for(uint32_t i = 0; i < barrelData.m_TileGamePosition.size(); ++i)
    {
        if(m_currentEntitiesDelete.find(barrelData.m_TileGamePosition[i]) !=
                m_currentEntitiesDelete.end())
        {
            continue;
        }
        uint32_t barrelEntity = createBarrelEntity();
        SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(barrelEntity);
        MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().getMemSpriteDataComponent(barrelEntity);
        GeneralCollisionComponent *genComp = m_ecsManager.getComponentManager().getGeneralCollisionComponent(barrelEntity);
        CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().getCircleCollComponent(barrelEntity);
        FPSVisibleStaticElementComponent *fpsComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(barrelEntity);
        BarrelComponent *barrelComp = m_ecsManager.getComponentManager().getBarrelComponent(barrelEntity);
        MemFPSGLSizeComponent *memGLSizeComp = m_ecsManager.getComponentManager().getMemFPSGLSizeComponent(barrelEntity);
        MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(barrelEntity);
        TimerComponent *timerComp = m_ecsManager.getComponentManager().getTimerComponent(barrelEntity);
        AudioComponent *audioComp = m_ecsManager.getComponentManager().getAudioComponent(barrelEntity);
        assert(audioComp);
        assert(timerComp);
        assert(mapComp);
        assert(memGLSizeComp);
        assert(barrelComp);
        assert(genComp);
        assert(fpsComp);
        assert(circleComp);
        assert(memSpriteComp);
        assert(spriteComp);
        if(!m_memSoundElements.m_barrels)
        {
            m_memSoundElements.m_barrels = loadSound(barrelData.m_explosionSoundFile);
        }
        audioComp->m_soundElements.push_back(*m_memSoundElements.m_barrels);
        mapComp->m_coord = barrelData.m_TileGamePosition[i];
        Level::addElementCase(spriteComp, barrelData.m_TileGamePosition[i], LevelCaseType_e::EMPTY_LC, barrelEntity);
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
        barrelComp->m_life = 3;
        barrelComp->m_memPosExplosionSprite = barrelData.m_staticSprite.size() - 1;
        barrelComp->m_damageZoneEntity = createDamageZoneEntity(15, CollisionTag_e::EXPLOSION_CT, 30.0f, levelManager.getHitSoundFile());
        timerComp->m_cycleCountA = 0;
    }
}

//===================================================================
SoundElement MainEngine::loadSound(const std::string &file)
{
    std::optional<ALuint> num = m_audioEngine.loadSoundEffectFromFile(file);
    if(!num)
    {
        std::cout << "loading " << file << " failed\n";
    }
    assert(num);
    return {m_audioEngine.getSoundSystem()->createSource(*num), *num, false};
}

//===================================================================
uint32_t MainEngine::loadDisplayTeleportEntity(const LevelManager &levelManager)
{
    uint32_t numEntity = createDisplayTeleportEntity();
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(numEntity);
    MemSpriteDataComponent *memSpriteComp = m_ecsManager.getComponentManager().getMemSpriteDataComponent(numEntity);
    PositionVertexComponent *posCursor = m_ecsManager.getComponentManager().getPosVertexComponent(numEntity);
    assert(posCursor);
    GeneralCollisionComponent *genColl = m_ecsManager.getComponentManager().getGeneralCollisionComponent(numEntity);
    assert(genColl);
    genColl->m_tagA = CollisionTag_e::GHOST_CT;
    genColl->m_tagB = CollisionTag_e::TELEPORT_ANIM_CT;
    genColl->m_active = false;
    posCursor->m_vertex.reserve(4);
    float up = 0.75f, down = -0.75f, left = -0.75f, right = 0.75f;
    posCursor->m_vertex.insert(posCursor->m_vertex.end(), {{left, up}, {right, up}, {right, down},{left, down}});
    assert(memSpriteComp);
    assert(spriteComp);
    const std::vector<SpriteData> &vectSprite = levelManager.getPictureData().getSpriteData();
    const std::vector<MemSpriteData> &visibleTeleportData = levelManager.getVisibleTeleportData();
    memSpriteComp->m_vectSpriteData.reserve(visibleTeleportData.size());
    for(uint32_t j = 0; j < visibleTeleportData.size(); ++j)
    {
        memSpriteComp->m_current = 0;
        memSpriteComp->m_vectSpriteData.emplace_back(&vectSprite[visibleTeleportData[j].m_numSprite]);
    }
    spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[0];
    m_graphicEngine.getVisionSystem().memTeleportAnimEntity(numEntity);
    return numEntity;
}

//===================================================================
bool MainEngine::loadExitElement(const LevelManager &levelManager,
                                 const StaticLevelElementData &exit)
{
    if(exit.m_TileGamePosition.empty())
    {
        return false;
    }
    const SpriteData &memSpriteData = levelManager.getPictureData().
            getSpriteData()[exit.m_numSprite];
    uint32_t entityNum = createStaticEntity();
    FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(entityNum);
    assert(fpsStaticComp);
    fpsStaticComp->m_inGameSpriteSize = exit.m_inGameSpriteSize;
    fpsStaticComp->m_levelElementType = LevelStaticElementType_e::CEILING;
    confBaseComponent(entityNum, memSpriteData, exit.m_TileGamePosition[0],
            CollisionShape_e::CIRCLE_C, CollisionTag_e::EXIT_CT);
    confStaticComponent(entityNum, exit.m_inGameSpriteSize, LevelStaticElementType_e::GROUND);
    CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().getCircleCollComponent(entityNum);
    assert(circleComp);
    circleComp->m_ray = 5.0f;
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(entityNum);
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(entityNum);
    assert(spriteComp);
    assert(mapComp);
    Level::addElementCase(spriteComp, mapComp->m_coord, LevelCaseType_e::EMPTY_LC, entityNum);
    return true;
}

//===================================================================
void MainEngine::loadStaticElementGroup(const std::vector<SpriteData> &vectSpriteData,
                                        const std::map<std::string, StaticLevelElementData> &staticData,
                                        LevelStaticElementType_e elementType, const std::string &soundFile)
{
    std::map<std::string, StaticLevelElementData>::const_iterator it = staticData.begin();
    for(; it != staticData.end(); ++it)
    {
        for(uint32_t j = 0; j < it->second.m_TileGamePosition.size(); ++j)
        {
            createStaticElementEntity(elementType, it->second, vectSpriteData, j, false, soundFile);
        }
    }
}

//===================================================================
std::optional<uint32_t> MainEngine::createStaticElementEntity(LevelStaticElementType_e elementType, const StaticLevelElementData &staticElementData,
                                                              const std::vector<SpriteData> &vectSpriteData, uint32_t iterationNum, bool enemyDrop, const std::string &soundFile)
{
    CollisionTag_e tag;
    uint32_t entityNum;
    const SpriteData &memSpriteData = vectSpriteData[staticElementData.m_numSprite];
    if(!enemyDrop && m_currentEntitiesDelete.find(staticElementData.m_TileGamePosition[iterationNum]) !=
            m_currentEntitiesDelete.end())
    {
        return {};
    }
    if(elementType == LevelStaticElementType_e::OBJECT)
    {
        tag = CollisionTag_e::OBJECT_CT;
        entityNum = confObjectEntity(staticElementData);
    }
    else if(elementType == LevelStaticElementType_e::TELEPORT)
    {
        tag = CollisionTag_e::TELEPORT_CT;
        entityNum = confTeleportEntity(staticElementData, iterationNum, soundFile);
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
    FPSVisibleStaticElementComponent *fpsStaticComp = m_ecsManager.getComponentManager().getFPSVisibleStaticElementComponent(entityNum);
    assert(fpsStaticComp);
    fpsStaticComp->m_inGameSpriteSize = staticElementData.m_inGameSpriteSize;
    fpsStaticComp->m_levelElementType = elementType;
    //Enemy dropable object case
    if(iterationNum >= staticElementData.m_TileGamePosition.size())
    {
        confBaseComponent(entityNum, memSpriteData, {}, CollisionShape_e::CIRCLE_C, tag);
    }
    else
    {
        confBaseComponent(entityNum, memSpriteData, staticElementData.m_TileGamePosition[iterationNum], CollisionShape_e::CIRCLE_C, tag);
    }
    CircleCollisionComponent *circleComp = m_ecsManager.getComponentManager().getCircleCollComponent(entityNum);
    assert(circleComp);
    circleComp->m_ray = staticElementData.m_inGameSpriteSize.first * LEVEL_THIRD_TILE_SIZE_PX;
    confStaticComponent(entityNum, staticElementData.m_inGameSpriteSize, elementType);
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(entityNum);
    assert(spriteComp);
    MapCoordComponent *mapComp = m_ecsManager.getComponentManager().getMapCoordComponent(entityNum);
    assert(mapComp);
    Level::addElementCase(spriteComp, mapComp->m_coord, LevelCaseType_e::EMPTY_LC, entityNum);
    return entityNum;
}

//===================================================================
uint32_t MainEngine::confTeleportEntity(const StaticLevelElementData &teleportData,
                                        uint32_t iterationNum,
                                        const std::string &soundFile)
{
    uint32_t entityNum = createTeleportEntity();
    TeleportComponent *teleportComp = m_ecsManager.getComponentManager().getTeleportComponent(entityNum);
    AudioComponent *audioComp = m_ecsManager.getComponentManager().getAudioComponent(entityNum);
    assert(teleportComp);
    assert(audioComp);
    if(!m_memSoundElements.m_teleports)
    {
        m_memSoundElements.m_teleports = loadSound(soundFile);
    }
    audioComp->m_soundElements.push_back(*m_memSoundElements.m_teleports);
    teleportComp->m_targetPos = teleportData.m_teleportData->m_targetTeleport[iterationNum];
    return entityNum;
}

//===================================================================
uint32_t MainEngine::confObjectEntity(const StaticLevelElementData &objectData)
{
    uint32_t entityNum = createObjectEntity();
    ObjectConfComponent *objComp = m_ecsManager.getComponentManager().getObjectConfComponent(entityNum);
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
        objComp->m_cardName = objectData.m_cardName;
        objComp->m_cardID = objectData.m_cardID;
    }
    return entityNum;
}


//===================================================================
void MainEngine::confColorBackgroundComponents(uint32_t entity, const GroundCeilingData &groundData, bool ground)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().getPosVertexComponent(entity);
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
    ColorVertexComponent *colorComp = m_ecsManager.getComponentManager().getColorVertexComponent(entity);
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
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().getPosVertexComponent(entity);
    assert(posComp);
    posComp->m_vertex.reserve(6);
    posComp->m_vertex.emplace_back(-1.0f, 0.0f);
    posComp->m_vertex.emplace_back(1.0f, 0.0f);
    posComp->m_vertex.emplace_back(1.0f, -1.0f);
    posComp->m_vertex.emplace_back(-1.0f, -1.0f);
    posComp->m_vertex.emplace_back(3.0f, 0.0f);
    posComp->m_vertex.emplace_back(3.0f, -1.0f);
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(entity);
    assert(spriteComp);
    assert(vectSprite.size() >= groundData.m_spriteSimpleTextNum);
    spriteComp->m_spriteData = &vectSprite[groundData.m_spriteSimpleTextNum];
}

//===================================================================
void MainEngine::confCeilingSimpleTextBackgroundComponents(uint32_t entity, const GroundCeilingData &groundData,
                                                           const std::vector<SpriteData> &vectSprite)
{
    PositionVertexComponent *posComp = m_ecsManager.getComponentManager().getPosVertexComponent(entity);
    assert(posComp);
    posComp->m_vertex.reserve(6);
    posComp->m_vertex.emplace_back(-1.0f, 1.0f);
    posComp->m_vertex.emplace_back(1.0f, 1.0f);
    posComp->m_vertex.emplace_back(1.0f, 0.0f);
    posComp->m_vertex.emplace_back(-1.0f, 0.0f);
    posComp->m_vertex.emplace_back(3.0f, 1.0f);
    posComp->m_vertex.emplace_back(3.0f, 0.0f);
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(entity);
    assert(spriteComp);
    assert(vectSprite.size() >= groundData.m_spriteSimpleTextNum);
    spriteComp->m_spriteData = &vectSprite[groundData.m_spriteSimpleTextNum];
}

//===================================================================
void MainEngine::confTiledTextBackgroundComponents(uint32_t entity, const GroundCeilingData &backgroundData,
                                                   const std::vector<SpriteData> &vectSprite)
{
    SpriteTextureComponent *spriteComp = m_ecsManager.getComponentManager().getSpriteTextureComponent(entity);
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
    staticDisplay->linkMainEngine(this);
    vision->memRefMainEngine(this);
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
    input->linkMainEngine(this);
    input->init(m_graphicEngine.getGLWindow());
    iaSystem->linkMainEngine(this);
    coll->linkMainEngine(this);
    door->memRefMainEngine(this);
    m_physicalEngine.linkSystems(input, coll, door, iaSystem);
}

//===================================================================
void MainEngine::linkSystemsToSoundEngine()
{
    SoundSystem *soundSystem = m_ecsManager.getSystemManager().
            searchSystemByType<SoundSystem>(static_cast<uint32_t>(Systems_e::SOUND_SYSTEM));
    m_audioEngine.linkSystem(soundSystem);
}

//===================================================================
float getDegreeAngleFromDirection(Direction_e direction)
{
    switch(direction)
    {
    case Direction_e::NORTH:
        return 90.0f;
    case Direction_e::EAST:
        return 0.0f;
    case Direction_e::SOUTH:
        return 270.0f;
    case Direction_e::WEST:
        return 180.0f;
    }
    return 0.0f;
}
