#include <cassert>
#include "VisionSystem.hpp"
#include <constants.hpp>
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
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
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
    GeneralCollisionComponent *collCompB;
    VisionComponent *visionCompA;
    MapCoordComponent *mapCompA;
    MoveableComponent *moveCompA;
    std::bitset<Components_e::TOTAL_COMPONENTS> bitsetComp;
    bitsetComp[Components_e::MAP_COORD_COMPONENT] = true;
    bitsetComp[Components_e::SPRITE_TEXTURE_COMPONENT] = true;
    bitsetComp[Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = true;
    std::vector<uint32_t> vectEntities = m_memECSManager->getEntityContainingComponents(bitsetComp);
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
            collCompB = stairwayToComponentManager().
                    searchComponentByType<GeneralCollisionComponent>(vectEntities[j],
                                          Components_e::GENERAL_COLLISION_COMPONENT);
            assert(collCompB);
            treatVisible(visionCompA, moveCompA, vectEntities[j]);
        }
        updateSprites(mVectNumEntity[i], vectEntities);
    }
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
        if(genComp->m_tag == CollisionTag_e::BULLET_ENEMY_CT ||
                genComp->m_tag == CollisionTag_e::BULLET_PLAYER_CT)
        {
            updateVisibleShotSprite(vectEntities[i], memSpriteComp, spriteComp, timerComp, genComp);
        }
        else if(genComp->m_tag == CollisionTag_e::ENEMY_CT ||
                genComp->m_tag == CollisionTag_e::GHOST_CT)
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
        else if(genComp->m_tag == CollisionTag_e::IMPACT_CT)
        {
            updateImpactSprites(vectEntities[i], memSpriteComp, spriteComp, timerComp, genComp);
        }
    }
}

//===========================================================================
void VisionSystem::updateVisibleShotSprite(uint32_t shotEntity,
                                           MemSpriteDataComponent *memSpriteComp,
                                           SpriteTextureComponent *spriteComp,
                                           TimerComponent *timerComp,
                                           GeneralCollisionComponent *genComp)
{
    ShotConfComponent *shotComp = stairwayToComponentManager().
            searchComponentByType<ShotConfComponent>(shotEntity,
                                                             Components_e::SHOT_CONF_COMPONENT);
    assert(shotComp);
    if(!shotComp->m_destructPhase)
    {
        return;
    }
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
            timerComp->m_clockB;
    uint32_t spriteIndex = static_cast<uint32_t>(shotComp->m_spritePhaseShot);
    if(elapsed_seconds.count() > 0.12)
    {
        if(shotComp->m_spritePhaseShot != ShotPhase_e::SHOT_DESTRUCT_C)
        {
            ++spriteIndex;
            timerComp->m_clockB = std::chrono::system_clock::now();
        }
        else
        {
            genComp->m_active = false;
            shotComp->m_destructPhase = false;
            shotComp->m_spritePhaseShot = ShotPhase_e::NORMAL;
            spriteComp->m_spriteData = memSpriteComp->
                    m_vectSpriteData[static_cast<uint32_t>(ShotPhase_e::NORMAL)];
            return;
        }
    }
    shotComp->m_spritePhaseShot = static_cast<ShotPhase_e>(spriteIndex);
    assert(static_cast<uint32_t>(ShotPhase_e::TOTAL) ==
           memSpriteComp->m_vectSpriteData.size());
    spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[spriteIndex];
}

//===========================================================================
void VisionSystem::updateEnemySprites(uint32_t enemyEntity, uint32_t observerEntity,
                                      MemSpriteDataComponent *memSpriteComp,
                                      SpriteTextureComponent *spriteComp,
                                      TimerComponent *timerComp, EnemyConfComponent *enemyConfComp)
{
    uint32_t indexSprite;
    if(enemyConfComp->m_touched)
    {
        spriteComp->m_spriteData = memSpriteComp->
                m_vectSpriteData[static_cast<uint32_t>(EnemySpriteType_e::TOUCHED)];
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
                timerComp->m_clockC;
        if(elapsed_seconds.count() > 0.2)
        {
            enemyConfComp->m_touched = false;
        }
    }
    else if(enemyConfComp->m_behaviourMode == EnemyBehaviourMode_e::ATTACK &&
            enemyConfComp->m_attackPhase == EnemyAttackPhase_e::SHOOT)
    {
        spriteComp->m_spriteData = memSpriteComp->
                m_vectSpriteData[static_cast<uint32_t>(EnemySpriteType_e::ATTACK_A)];
        return;
    }
    else if(enemyConfComp->m_displayMode == EnemyDisplayMode_e::NORMAL)
    {
        if(enemyConfComp->m_life == 0)
        {
            enemyConfComp->m_displayMode = EnemyDisplayMode_e::DYING;
            spriteComp->m_spriteData = memSpriteComp->
                    m_vectSpriteData[static_cast<uint32_t>(EnemySpriteType_e::DYING)];
            timerComp->m_clockA = std::chrono::system_clock::now();
        }
        else
        {
            MoveableComponent *enemyMoveComp = stairwayToComponentManager().
                    searchComponentByType<MoveableComponent>(enemyEntity,
                                                             Components_e::MOVEABLE_COMPONENT);
            assert(enemyMoveComp);
            enemyConfComp->m_visibleOrientation =
                    getOrientationFromAngle(observerEntity, enemyEntity,
                                            enemyMoveComp->m_degreeOrientation);

            indexSprite = static_cast<uint32_t>(enemyConfComp->m_visibleOrientation);
            std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - timerComp->m_clockA;
            if(elapsed_seconds.count() > 0.5)
            {
                enemyConfComp->m_staticPhase = (enemyConfComp->m_staticPhase) ? false : true;
                timerComp->m_clockA = std::chrono::system_clock::now();
            }
            if(!enemyConfComp->m_staticPhase)
            {
                ++indexSprite;
            }
            spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[indexSprite];
        }
    }
    else if(enemyConfComp->m_displayMode == EnemyDisplayMode_e::DYING)
    {
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
                timerComp->m_clockA;
        if(elapsed_seconds.count() > 0.5)
        {
            enemyConfComp->m_displayMode = EnemyDisplayMode_e::DEAD;
            spriteComp->m_spriteData = memSpriteComp->
                    m_vectSpriteData[static_cast<uint32_t>(EnemySpriteType_e::DEAD)];
        }
    }
}

//===========================================================================
void VisionSystem::updateImpactSprites(uint32_t entityImpact, MemSpriteDataComponent *memSpriteComp,
                                       SpriteTextureComponent *spriteComp,
                                       TimerComponent *timerComp,
                                       GeneralCollisionComponent *genComp)
{
    ImpactShotComponent *impactComp = stairwayToComponentManager().
            searchComponentByType<ImpactShotComponent>(entityImpact,
                                                       Components_e::IMPACT_CONF_COMPONENT);
    assert(impactComp);
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
            timerComp->m_clockA;
    impactComp->m_moveUp += 0.02f;
    if(elapsed_seconds.count() > 0.20)
    {
        if(impactComp->m_spritePhase == ImpactPhase_e::FIRST)
        {
            uint32_t current = static_cast<uint32_t>(impactComp->m_spritePhase);
            impactComp->m_spritePhase = static_cast<ImpactPhase_e>(++current);
            spriteComp->m_spriteData = memSpriteComp->m_vectSpriteData[current];
            timerComp->m_clockA = std::chrono::system_clock::now();
        }
        else
        {
            genComp->m_active = false;
            impactComp->m_moveUp = EPSILON_FLOAT;
        }
    }
}

//===========================================================================
EnemySpriteType_e VisionSystem::getOrientationFromAngle(uint32_t observerEntity,
                                                        uint32_t targetEntity,
                                                        float targetDegreeAngle)
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
        return EnemySpriteType_e::STATIC_MID_BACK_RIGHT_A;
    }
    else if(valCos < -0.333f && valSin > 0.333f)
    {
        return EnemySpriteType_e::STATIC_MID_FRONT_RIGHT_A;
    }
    else if(valCos > 0.333f && valSin < -0.333f)
    {
        return EnemySpriteType_e::STATIC_MID_BACK_LEFT_A;
    }
    else if(valCos < -0.333f && valSin < -0.333f)
    {
        return EnemySpriteType_e::STATIC_MID_FRONT_LEFT_A;
    }
    else if(valSin < -0.5f)
    {
        return EnemySpriteType_e::STATIC_LEFT_A;
    }
    else if(valSin > 0.5f)
    {
        return EnemySpriteType_e::STATIC_RIGHT_A;
    }
    else if(valCos < -0.5f)
    {
        return EnemySpriteType_e::STATIC_FRONT_A;
    }
    else
    {
        return EnemySpriteType_e::STATIC_BACK_A;
    }
}

//===========================================================================
void VisionSystem::treatVisible(VisionComponent *visionComp, MoveableComponent *moveCompA,
                                uint32_t numEntity)
{
    MapCoordComponent *mapCompB = stairwayToComponentManager().
            searchComponentByType<MapCoordComponent>(numEntity, Components_e::MAP_COORD_COMPONENT);
    assert(visionComp);
    assert(mapCompB);
    float angleElement = getTrigoAngle(std::get<0>(visionComp->m_triangleVision),
                                       mapCompB->m_absoluteMapPositionPX),
            diffAngle = std::abs(angleElement - moveCompA->m_degreeOrientation);
    if(diffAngle < HALF_CONE_VISION + 30.0f || diffAngle > 275.0f)
    {
        visionComp->m_vectVisibleEntities.push_back(numEntity);
    }
}

//===========================================================================
void updateTriangleVisionFromPosition(VisionComponent *visionComp, const MapCoordComponent *mapComp,
                                      const MoveableComponent *movComp)
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
