#include <cassert>
#include "VisionSystem.hpp"
#include <constants.hpp>
#include <MainEngine.hpp>
#include <CollisionUtils.hpp>
#include <PhysicalEngine.hpp>
#include <ECS/ECSManager.hpp>
#include <math.h>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/FPSVisibleStaticElementComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/WallMultiSpriteConf.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/MemFPSGLSizeComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/BarrelComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/ShotConfComponent.hpp>
#include <ECS/Components/ImpactShotComponent.hpp>

//===========================================================================
VisionSystem::VisionSystem(const ECSManager *memECSManager) :
    m_memECSManager(memECSManager)
{
    setUsedComponents();
}

//===========================================================================
void VisionSystem::setUsedComponents()
{
    bAddComponentToSystem(Components_e::VISION_COMPONENT);
}

//===========================================================================
void VisionSystem::execSystem()
{
    System::execSystem();
    VisionComponent *visionCompA;
    MapCoordComponent *mapCompA;
    MoveableComponent *moveCompA;
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComp;
    bitsetComp[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComp[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComp[Components_e::GENERAL_COLLISION_COMPONENT] = true;
    bitsetComp[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    std::vector<uint32_t> vectEntities = m_memECSManager->getEntitiesContainingComponents(bitsetComp);
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        visionCompA = stairwayToComponentManager().
                searchComponentByType<VisionComponent>(mVectNumEntity[i], Components_e::VISION_COMPONENT);
        mapCompA = stairwayToComponentManager().
                searchComponentByType<MapCoordComponent>(mVectNumEntity[i], Components_e::MAP_COORD_COMPONENT);
        moveCompA = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(mVectNumEntity[i], Components_e::MOVEABLE_COMPONENT);
        updateTriangleVisionFromPosition(visionCompA, mapCompA, moveCompA);
        visionCompA->m_vectVisibleEntities.clear();
        for(uint32_t j = 0; j < vectEntities.size(); ++j)
        {
            if(mVectNumEntity[i] == vectEntities[j])
            {
                continue;
            }
            treatVisible(visionCompA, moveCompA, vectEntities[j]);
        }
        updateSprites(mVectNumEntity[i], vectEntities);
    }
    updateWallSprites();
    updateTeleportAnimationSprites();
}

//===========================================================================
void VisionSystem::memRefMainEngine(MainEngine *mainEngine)
{
    m_refMainEngine = mainEngine;
}

//===========================================================================
void VisionSystem::updateSprites(uint32_t observerEntity,
                                 const std::vector<uint32_t> &vectEntities)
{
    MemSpriteDataComponent *memSpriteComp;
    GeneralCollisionComponent *genComp;
    SpriteTextureComponent *spriteComp;
    TimerComponent *timerComp;
    for(uint32_t i = 0; i < vectEntities.size(); ++i)
    {
        memSpriteComp = stairwayToComponentManager().
                searchComponentByType<MemSpriteDataComponent>(vectEntities[i],
                                                              Components_e::MEM_SPRITE_DATA_COMPONENT);
        if(!memSpriteComp)
        {
            continue;
        }
        genComp = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(vectEntities[i],
                                                                 Components_e::GENERAL_COLLISION_COMPONENT);
        assert(genComp);
        if(!genComp->m_active)
        {
            continue;
        }
        spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(vectEntities[i],
                                                              Components_e::SPRITE_TEXTURE_COMPONENT);
        timerComp = stairwayToComponentManager().
                searchComponentByType<TimerComponent>(vectEntities[i], Components_e::TIMER_COMPONENT);
        assert(spriteComp);
        assert(timerComp);
        if(genComp->m_tagA == CollisionTag_e::BULLET_ENEMY_CT ||
                genComp->m_tagA == CollisionTag_e::BULLET_PLAYER_CT)
        {
            updateVisibleShotSprite(vectEntities[i], memSpriteComp, spriteComp, timerComp, genComp);
        }
        else if(genComp->m_tagA == CollisionTag_e::BARREL_CT)
        {
            updateBarrelSprite(vectEntities[i], memSpriteComp, spriteComp, timerComp, genComp);
        }
        else if(genComp->m_tagB == CollisionTag_e::IMPACT_CT)
        {
            updateImpactSprites(vectEntities[i], memSpriteComp, spriteComp, timerComp, genComp);
        }
        //OOOOK put enemy tag to tagB
        else if(genComp->m_tagA == CollisionTag_e::ENEMY_CT || genComp->m_tagA == CollisionTag_e::GHOST_CT)
        {
            EnemyConfComponent *enemyConfComp = stairwayToComponentManager().
                    searchComponentByType<EnemyConfComponent>(vectEntities[i],
                                                              Components_e::ENEMY_CONF_COMPONENT);
            if(enemyConfComp)
            {
                updateEnemySprites(vectEntities[i], observerEntity,
                                   memSpriteComp, spriteComp, timerComp, enemyConfComp);
            }
        }
    }
}

//===========================================================================
void VisionSystem::updateWallSprites()
{
    if(m_memMultiSpritesWallEntities.empty())
    {
        memMultiSpritesWallEntities();
    }
    MemSpriteDataComponent *memSpriteComp;
    SpriteTextureComponent *spriteComp;
    TimerComponent *timerComp;
    float currentInterval;
    for(uint32_t i = 0; i < m_memMultiSpritesWallEntities.size(); ++i)
    {
        spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(m_memMultiSpritesWallEntities[i],
                                                              Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        timerComp = stairwayToComponentManager().
                searchComponentByType<TimerComponent>(m_memMultiSpritesWallEntities[i], Components_e::TIMER_COMPONENT);
        assert(timerComp);
        memSpriteComp = stairwayToComponentManager().
                searchComponentByType<MemSpriteDataComponent>(m_memMultiSpritesWallEntities[i], Components_e::MEM_SPRITE_DATA_COMPONENT);
        assert(memSpriteComp);
        WallMultiSpriteConf *multiSpriteConf = stairwayToComponentManager().
                searchComponentByType<WallMultiSpriteConf>(m_memMultiSpritesWallEntities[i], Components_e::WALL_MULTI_SPRITE_CONF);
        assert(multiSpriteConf);
        if(!multiSpriteConf->m_cyclesTime.empty())
        {
            assert(memSpriteComp->m_current < multiSpriteConf->m_cyclesTime.size());
            currentInterval = multiSpriteConf->m_cyclesTime[memSpriteComp->m_current];
        }
        else
        {
            currentInterval = m_defaultInterval;
        }
        if(++timerComp->m_cycleCountA >= currentInterval)
        {
            ++memSpriteComp->m_current;
            if(memSpriteComp->m_current >= memSpriteComp->m_vectSpriteData.size())
            {
                memSpriteComp->m_current = 0;
            }
            spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[memSpriteComp->m_current];
            timerComp->m_cycleCountA = 0;
        }
    }
}

//===========================================================================
void VisionSystem::updateTeleportAnimationSprites()
{
    MemSpriteDataComponent *memSpriteComp = stairwayToComponentManager().
            searchComponentByType<MemSpriteDataComponent>(m_memTeleportAnimEntity, Components_e::MEM_SPRITE_DATA_COMPONENT);
    assert(memSpriteComp);
    TimerComponent *timerComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(m_memTeleportAnimEntity, Components_e::TIMER_COMPONENT);
    assert(timerComp);
    GeneralCollisionComponent *genComp = stairwayToComponentManager().
            searchComponentByType<GeneralCollisionComponent>(m_memTeleportAnimEntity, Components_e::GENERAL_COLLISION_COMPONENT);
    assert(genComp);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(m_memTeleportAnimEntity, Components_e::SPRITE_TEXTURE_COMPONENT);
    assert(spriteComp);
    if(++timerComp->m_cycleCountA >= m_teleportIntervalTime.second)
    {
        genComp->m_active = false;
    }
    else if(++timerComp->m_cycleCountB >= m_teleportIntervalTime.first)
    {
        timerComp->m_cycleCountB = 0;
        if(memSpriteComp->m_current < memSpriteComp->m_vectSpriteData.size() - 1)
        {
            ++memSpriteComp->m_current;
        }
        else
        {
            memSpriteComp->m_current = 0;
        }
        spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[memSpriteComp->m_current];
    }
}

//===========================================================================
void VisionSystem::memMultiSpritesWallEntities()
{
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComp;
    bitsetComp[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComp[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComp[Components_e::MEM_SPRITE_DATA_COMPONENT] = true;
    bitsetComp[Components_e::TIMER_COMPONENT] = true;
    std::vector<uint32_t> vectEntities = m_memECSManager->
            getEntitiesContainingComponents(bitsetComp);
    GeneralCollisionComponent *genComp;
    for(uint32_t i = 0; i < vectEntities.size(); ++i)
    {
        genComp = stairwayToComponentManager().
                searchComponentByType<GeneralCollisionComponent>(vectEntities[i], Components_e::GENERAL_COLLISION_COMPONENT);
        assert(genComp);
        if(genComp->m_tagA == CollisionTag_e::WALL_CT)
        {
            m_memMultiSpritesWallEntities.push_back(vectEntities[i]);
        }
    }
}

//===========================================================================
void VisionSystem::updateVisibleShotSprite(uint32_t shotEntity, MemSpriteDataComponent *memSpriteComp, SpriteTextureComponent *spriteComp,
                                           TimerComponent *timerComp, GeneralCollisionComponent *genComp)
{
    ShotConfComponent *shotComp = stairwayToComponentManager().
            searchComponentByType<ShotConfComponent>(shotEntity, Components_e::SHOT_CONF_COMPONENT);
    assert(shotComp);
    if(!shotComp->m_destructPhase)
    {
        return;
    }
    MemFPSGLSizeComponent *memGLSizeComp = stairwayToComponentManager().
            searchComponentByType<MemFPSGLSizeComponent>(shotEntity, Components_e::MEM_FPS_GLSIZE_COMPONENT);
    FPSVisibleStaticElementComponent *fpsStaticComp = stairwayToComponentManager().
            searchComponentByType<FPSVisibleStaticElementComponent>(shotEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(fpsStaticComp);
    assert(memGLSizeComp);
    if(++timerComp->m_cycleCountA >= shotComp->m_cycleDestructNumber)
    {
        timerComp->m_cycleCountA = 0;
        if(shotComp->m_spriteShotNum != memSpriteComp->m_vectSpriteData.size() - 1)
        {
            ++shotComp->m_spriteShotNum;
        }
        else
        {
            genComp->m_active = false;
            shotComp->m_destructPhase = false;
            shotComp->m_spriteShotNum = 0;
        }
    }
    assert(shotComp->m_spriteShotNum < memGLSizeComp->m_memGLSizeData.size());
    fpsStaticComp->m_inGameSpriteSize = memGLSizeComp->m_memGLSizeData[shotComp->m_spriteShotNum];
    spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[shotComp->m_spriteShotNum];
}

//===========================================================================
void VisionSystem::updateBarrelSprite(uint32_t barrelEntity, MemSpriteDataComponent *memSpriteComp,
                                      SpriteTextureComponent *spriteComp, TimerComponent *timerComp, GeneralCollisionComponent *genComp)
{
    BarrelComponent *barrelComp = stairwayToComponentManager().
            searchComponentByType<BarrelComponent>(barrelEntity, Components_e::BARREL_COMPONENT);
    assert(barrelComp);
    MemFPSGLSizeComponent *glSizeComp = stairwayToComponentManager().
            searchComponentByType<MemFPSGLSizeComponent>(barrelEntity, Components_e::MEM_FPS_GLSIZE_COMPONENT);
    assert(glSizeComp);
    FPSVisibleStaticElementComponent *fpsComp = stairwayToComponentManager().
            searchComponentByType<FPSVisibleStaticElementComponent>(barrelEntity, Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT);
    assert(fpsComp);
    if(!barrelComp->m_destructPhase)
    {
        if(++timerComp->m_cycleCountA >= barrelComp->m_timeStaticPhase)
        {
            if(memSpriteComp->m_current < barrelComp->m_memPosExplosionSprite)
            {
                ++memSpriteComp->m_current;
            }
            else
            {
                memSpriteComp->m_current = 0;
            }
            spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[memSpriteComp->m_current];
            timerComp->m_cycleCountA = 0;
        }
    }
    else
    {
        //first sprite destruct
        if(memSpriteComp->m_current <= barrelComp->m_memPosExplosionSprite)
        {
            timerComp->m_cycleCountA = 0;
            memSpriteComp->m_current = barrelComp->m_memPosExplosionSprite + 1;
            spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[memSpriteComp->m_current];
            fpsComp->m_inGameSpriteSize = glSizeComp->m_memGLSizeData[memSpriteComp->m_current];
            return;
        }
        if(++timerComp->m_cycleCountA >= barrelComp->m_timeStaticPhase)
        {
            timerComp->m_cycleCountA = 0;
            if(memSpriteComp->m_current != memSpriteComp->m_vectSpriteData.size() - 1)
            {
                ++memSpriteComp->m_current;
                spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[memSpriteComp->m_current];
                fpsComp->m_inGameSpriteSize = glSizeComp->m_memGLSizeData[memSpriteComp->m_current];
                if(memSpriteComp->m_current == barrelComp->m_memPosExplosionSprite + 2)
                {
                    MapCoordComponent *mapCompA = stairwayToComponentManager().
                            searchComponentByType<MapCoordComponent>(barrelEntity, Components_e::MAP_COORD_COMPONENT);
                    assert(mapCompA);
                    //active damage zone
                    GeneralCollisionComponent *genComp = stairwayToComponentManager().
                            searchComponentByType<GeneralCollisionComponent>(barrelComp->m_damageZoneEntity, Components_e::GENERAL_COLLISION_COMPONENT);
                    assert(genComp);
                    MapCoordComponent *mapCompB = stairwayToComponentManager().
                            searchComponentByType<MapCoordComponent>(barrelComp->m_damageZoneEntity, Components_e::MAP_COORD_COMPONENT);
                    assert(mapCompB);
                    GeneralCollisionComponent *collComp = stairwayToComponentManager().
                            searchComponentByType<GeneralCollisionComponent>(barrelEntity, Components_e::GENERAL_COLLISION_COMPONENT);
                    assert(collComp);
                    collComp->m_tagB = CollisionTag_e::GHOST_CT;
                    mapCompB->m_absoluteMapPositionPX = mapCompA->m_absoluteMapPositionPX;
                    genComp->m_active = true;
                }
            }
            //end animation remove entity
            else
            {
                m_vectBarrelsEntitiesToDelete.push_back(barrelEntity);
                genComp->m_active = false;
            }
        }
    }
}

//===========================================================================
void VisionSystem::updateEnemySprites(uint32_t enemyEntity, uint32_t observerEntity,
                                      MemSpriteDataComponent *memSpriteComp,
                                      SpriteTextureComponent *spriteComp,
                                      TimerComponent *timerComp, EnemyConfComponent *enemyConfComp)
{
    if(enemyConfComp->m_touched)
    {
        if(enemyConfComp->m_frozenOnAttack)
        {
            enemyConfComp->m_currentSprite =
                    enemyConfComp->m_mapSpriteAssociate.find(EnemySpriteType_e::TOUCHED)->second.first;
        }
        if(++timerComp->m_cycleCountC >= enemyConfComp->m_cycleNumberSpriteUpdate)
        {
            enemyConfComp->m_touched = false;
        }
    }
    else if(enemyConfComp->m_behaviourMode == EnemyBehaviourMode_e::ATTACK &&
            enemyConfComp->m_attackPhase == EnemyAttackPhase_e::SHOOT)
    {
        updateEnemyAttackSprite(enemyConfComp, timerComp);
    }
    else if(enemyConfComp->m_displayMode == EnemyDisplayMode_e::NORMAL)
    {
        updateEnemyNormalSprite(enemyConfComp, timerComp, enemyEntity, observerEntity);
    }
    else if(enemyConfComp->m_displayMode == EnemyDisplayMode_e::DYING)
    {
        mapEnemySprite_t::const_iterator it = enemyConfComp->m_mapSpriteAssociate.find(EnemySpriteType_e::DYING);
        if(enemyConfComp->m_currentSprite == it->second.second)
        {
            enemyConfComp->m_displayMode = EnemyDisplayMode_e::DEAD;
            if(enemyConfComp->m_endLevel)
            {
                m_refMainEngine->activeEndLevel();
            }
        }
        else if(++timerComp->m_cycleCountB >= enemyConfComp->m_cycleNumberDyingInterval)
        {
            ++enemyConfComp->m_currentSprite;
            timerComp->m_cycleCountB = 0;
        }
    }
    spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[static_cast<uint32_t>(enemyConfComp->m_currentSprite)];
}

//===========================================================================
void VisionSystem::updateEnemyNormalSprite(EnemyConfComponent *enemyConfComp, TimerComponent *timerComp,
                                           uint32_t enemyEntity, uint32_t observerEntity)
{
    if(enemyConfComp->m_behaviourMode == EnemyBehaviourMode_e::DYING)
    {
        enemyConfComp->m_displayMode = EnemyDisplayMode_e::DYING;
        enemyConfComp->m_currentSprite = enemyConfComp->m_mapSpriteAssociate.find(EnemySpriteType_e::DYING)->second.first;
        timerComp->m_cycleCountA = 0;
        timerComp->m_cycleCountB = 0;
    }
    else
    {
        MoveableComponent *enemyMoveComp = stairwayToComponentManager().
                searchComponentByType<MoveableComponent>(enemyEntity,
                                                         Components_e::MOVEABLE_COMPONENT);
        assert(enemyMoveComp);
        EnemySpriteType_e currentOrientationSprite =
                getOrientationFromAngle(observerEntity, enemyEntity,
                                        enemyMoveComp->m_degreeOrientation);
        mapEnemySprite_t::const_iterator it = enemyConfComp->m_mapSpriteAssociate.find(currentOrientationSprite);
        //if sprite outside
        if(enemyConfComp->m_currentSprite < it->second.first ||
                enemyConfComp->m_currentSprite > it->second.second)
        {
            enemyConfComp->m_currentSprite = it->second.first;
            timerComp->m_cycleCountA = 0;
        }
        else if(++timerComp->m_cycleCountA > enemyConfComp->m_standardSpriteInterval)
        {
            if(enemyConfComp->m_currentSprite == it->second.second)
            {
                enemyConfComp->m_currentSprite = it->second.first;
            }
            else
            {
                ++enemyConfComp->m_currentSprite;
            }
            timerComp->m_cycleCountA = 0;
        }
    }
}

//===========================================================================
void updateEnemyAttackSprite(EnemyConfComponent *enemyConfComp, TimerComponent *timerComp)
{
    //first element
    mapEnemySprite_t::const_iterator it = enemyConfComp->m_mapSpriteAssociate.find(EnemySpriteType_e::ATTACK);
    //if last animation
    if(enemyConfComp->m_currentSprite == it->second.second)
    {
        return;
    }
    if(enemyConfComp->m_currentSprite >= it->second.first &&
            enemyConfComp->m_currentSprite <= it->second.second)
    {        
        if(++timerComp->m_cycleCountC >= enemyConfComp->m_cycleNumberAttackInterval)
        {
            ++enemyConfComp->m_currentSprite;
            timerComp->m_cycleCountC = 0;
        }
    }
    //if sprite is not ATTACK Go to First atack sprite
    else
    {
        enemyConfComp->m_currentSprite = enemyConfComp->m_mapSpriteAssociate.find(EnemySpriteType_e::ATTACK)->second.first;
        timerComp->m_cycleCountC = 0;
    }
}

//===========================================================================
void VisionSystem::updateImpactSprites(uint32_t entityImpact, MemSpriteDataComponent *memSpriteComp, SpriteTextureComponent *spriteComp,
                                       TimerComponent *timerComp, GeneralCollisionComponent *genComp)
{
    ImpactShotComponent *impactComp = stairwayToComponentManager().
            searchComponentByType<ImpactShotComponent>(entityImpact, Components_e::IMPACT_CONF_COMPONENT);
    assert(impactComp);
    if(!impactComp->m_touched)
    {
        impactComp->m_moveUp += 0.02f;
    }
    else
    {
        impactComp->m_moveUp -= 0.02f;
    }
    if(++timerComp->m_cycleCountA >= impactComp->m_intervalTime)
    {
        if(impactComp->m_spritePhase == ImpactPhase_e::FIRST)
        {
            uint32_t current = static_cast<uint32_t>(impactComp->m_spritePhase);
            impactComp->m_spritePhase = static_cast<ImpactPhase_e>(++current);
            spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[current];
            timerComp->m_cycleCountA = 0;
        }
        else
        {
            genComp->m_active = false;
        }
    }
}

//===========================================================================
EnemySpriteType_e VisionSystem::getOrientationFromAngle(uint32_t observerEntity, uint32_t targetEntity, float targetDegreeAngle)
{
    MapCoordComponent *observMapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(observerEntity,
                                                     Components_e::MAP_COORD_COMPONENT);
    MapCoordComponent *targetMapComp = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(targetEntity,
                                                     Components_e::MAP_COORD_COMPONENT);
    assert(observMapComp);
    assert(targetMapComp);
    float observerDegreeAngle = getTrigoAngle(observMapComp->m_absoluteMapPositionPX,
                                              targetMapComp->m_absoluteMapPositionPX);
    float radDiff = getRadiantAngle(observerDegreeAngle - targetDegreeAngle),
            valSin = std::sin(radDiff), valCos = std::cos(radDiff);
    if(valCos > 0.333f && valSin > 0.333f)
    {
        return EnemySpriteType_e::STATIC_BACK_RIGHT;
    }
    else if(valCos < -0.333f && valSin > 0.333f)
    {
        return EnemySpriteType_e::STATIC_FRONT_RIGHT;
    }
    else if(valCos > 0.333f && valSin < -0.333f)
    {
        return EnemySpriteType_e::STATIC_BACK_LEFT;
    }
    else if(valCos < -0.333f && valSin < -0.333f)
    {
        return EnemySpriteType_e::STATIC_FRONT_LEFT;
    }
    else if(valSin < -0.5f)
    {
        return EnemySpriteType_e::STATIC_LEFT;
    }
    else if(valSin > 0.5f)
    {
        return EnemySpriteType_e::STATIC_RIGHT;
    }
    else if(valCos < -0.5f)
    {
        return EnemySpriteType_e::STATIC_FRONT;
    }
    else
    {
        return EnemySpriteType_e::STATIC_BACK;
    }
}

//===========================================================================
void VisionSystem::treatVisible(VisionComponent *visionComp, MoveableComponent *moveCompA, uint32_t numEntity)
{
    MapCoordComponent *mapCompB = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(numEntity, Components_e::MAP_COORD_COMPONENT);
    assert(visionComp);
    assert(mapCompB);
    float angleElement = getTrigoAngle(std::get<0>(visionComp->m_triangleVision),
                                       mapCompB->m_absoluteMapPositionPX),
            diffAngle = std::abs(angleElement - moveCompA->m_degreeOrientation);
    if(diffAngle < HALF_CONE_VISION + 30.0f || diffAngle > 270.0f)
    {
        //peripheric view
        if(diffAngle > 30.0f)
        {

        }
        std::cerr << diffAngle << " DIFFFFFF\n";
        visionComp->m_vectVisibleEntities.push_back({numEntity, (diffAngle > 30.0f)});
    }
}

//===========================================================================
void updateTriangleVisionFromPosition(VisionComponent *visionComp, const MapCoordComponent *mapComp, const MoveableComponent *movComp)
{
    assert(visionComp);
    assert(mapComp);
    assert(movComp);
    visionComp->m_triangleVision[0] = mapComp->m_absoluteMapPositionPX;
    visionComp->m_triangleVision[1] = mapComp->m_absoluteMapPositionPX;
    visionComp->m_triangleVision[2] = mapComp->m_absoluteMapPositionPX;
    //second point of view
    float angleDegree = movComp->m_degreeOrientation - (HALF_CONE_VISION + 10.0f);//QUICK FIX
    float radiantAngle;
    for(uint32_t i = 1; i < 3; ++i)
    {
        radiantAngle = getRadiantAngle(angleDegree);
        visionComp->m_triangleVision[i].first += cos(radiantAngle) * visionComp->m_distanceVisibility;
        visionComp->m_triangleVision[i].second -= sin(radiantAngle) * visionComp->m_distanceVisibility;
        if(i == 2)
        {
            break;
        }
        angleDegree += (CONE_VISION + 20.0f);//QUICK FIX
    }
}

//===========================================================================
mapEnemySprite_t::const_reverse_iterator findMapLastElement(const mapEnemySprite_t &map,
                                                            EnemySpriteType_e key)
{
    for(mapEnemySprite_t::const_reverse_iterator rit = map.rbegin();
        rit != map.rend(); ++rit)
    {
        if(rit->first == key)
        {
            return rit;
        }
    }
    return map.rend();
}
