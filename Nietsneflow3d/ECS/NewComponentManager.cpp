#include "NewComponentManager.hpp"

//===================================================================
NewComponentManager::NewComponentManager()
{
    init();
}

//===================================================================
void NewComponentManager::init()
{
    m_componentsGroup.m_vectAudioComp.reserve(100);
    m_componentsGroup.m_vectBarrelComp.reserve(100);
    m_componentsGroup.m_vectCheckpointComp.reserve(100);
    m_componentsGroup.m_vectCircleCollisionComp.reserve(100);
    m_componentsGroup.m_vectColorVertexComp.reserve(100);
    m_componentsGroup.m_vectDoorComp.reserve(100);
    m_componentsGroup.m_vectEnemyConfComp.reserve(100);
    m_componentsGroup.m_vectFPSVisibleStaticElementComp.reserve(100);
    m_componentsGroup.m_vectGeneralCollisionComp.reserve(100);
    m_componentsGroup.m_vectImpactShotComp.reserve(100);
    m_componentsGroup.m_vectLogComp.reserve(100);
    m_componentsGroup.m_vectMapCoordComp.reserve(100);
    m_componentsGroup.m_vectMemFPSGLSizeComp.reserve(100);
    m_componentsGroup.m_vectMemPositionsVertexComp.reserve(100);
    m_componentsGroup.m_vectMemSpriteDataComp.reserve(100);
    m_componentsGroup.m_vectMoveableComp.reserve(100);
    m_componentsGroup.m_vectMoveableWallConfComp.reserve(100);
    m_componentsGroup.m_vectObjectConfComp.reserve(100);
    m_componentsGroup.m_vectPositionVertexComp.reserve(100);
    m_componentsGroup.m_vectRectangleCollisionComp.reserve(100);
    m_componentsGroup.m_vectSegmentCollisionComp.reserve(100);
    m_componentsGroup.m_vectShotConfComp.reserve(100);
    m_componentsGroup.m_vectSpriteTextureComp.reserve(100);
    m_componentsGroup.m_vectTeleportComp.reserve(100);
    m_componentsGroup.m_vectTimerComp.reserve(100);
    m_componentsGroup.m_vectTriggerComp.reserve(100);
    m_componentsGroup.m_vectVisionComp.reserve(100);
    m_componentsGroup.m_vectWallMultiSpriteConfComp.reserve(100);
    m_componentsGroup.m_vectWeaponComp.reserve(100);
    m_componentsGroup.m_vectWriteComp.reserve(100);
    m_componentsGroup.m_vectInputComp.reserve(100);
    m_componentsGroup.m_vectPlayerConfComp.reserve(100);
    m_refComponents.reserve(100);
}

//===================================================================
void NewComponentManager::addEntity(uint32_t numEntity, const std::bitset<TOTAL_COMPONENTS> &bitsetComponents)
{
    if(numEntity >= m_refComponents.size())
    {
        m_refComponents.resize(numEntity + 1);
    }
    for(uint32_t i = 0; i < bitsetComponents.size(); ++i)
    {
        if(bitsetComponents[i])
        {
            switch (i)
            {
            case Components_e::POSITION_VERTEX_COMPONENT:
                m_refComponents[numEntity][Components_e::POSITION_VERTEX_COMPONENT] = m_componentsGroup.m_vectPositionVertexComp.size();
                m_componentsGroup.m_vectPositionVertexComp.emplace_back(PositionVertexComponent());
                break;
            case Components_e::SPRITE_TEXTURE_COMPONENT:
                m_refComponents[numEntity][Components_e::SPRITE_TEXTURE_COMPONENT] = m_componentsGroup.m_vectSpriteTextureComp.size();
                m_componentsGroup.m_vectSpriteTextureComp.emplace_back(SpriteTextureComponent());
                break;
            case Components_e::MEM_SPRITE_DATA_COMPONENT:
                m_refComponents[numEntity][Components_e::MEM_SPRITE_DATA_COMPONENT] = m_componentsGroup.m_vectMemSpriteDataComp.size();
                m_componentsGroup.m_vectMemSpriteDataComp.emplace_back(MemSpriteDataComponent());
                break;
            case Components_e::COLOR_VERTEX_COMPONENT:
                m_refComponents[numEntity][Components_e::COLOR_VERTEX_COMPONENT] = m_componentsGroup.m_vectColorVertexComp.size();
                m_componentsGroup.m_vectColorVertexComp.emplace_back(ColorVertexComponent());
                break;
            case Components_e::MAP_COORD_COMPONENT:
                m_refComponents[numEntity][Components_e::MAP_COORD_COMPONENT] = m_componentsGroup.m_vectMapCoordComp.size();
                m_componentsGroup.m_vectMapCoordComp.emplace_back(MapCoordComponent());
                break;
            case Components_e::INPUT_COMPONENT:
                m_refComponents[numEntity][Components_e::INPUT_COMPONENT] = m_componentsGroup.m_vectInputComp.size();
                m_componentsGroup.m_vectInputComp.emplace_back(InputComponent());
                break;
            case Components_e::CIRCLE_COLLISION_COMPONENT:
                m_refComponents[numEntity][Components_e::CIRCLE_COLLISION_COMPONENT] = m_componentsGroup.m_vectCircleCollisionComp.size();
                m_componentsGroup.m_vectCircleCollisionComp.emplace_back(CircleCollisionComponent());
                break;
            case Components_e::SEGMENT_COLLISION_COMPONENT:
                m_refComponents[numEntity][Components_e::SEGMENT_COLLISION_COMPONENT] = m_componentsGroup.m_vectSegmentCollisionComp.size();
                m_componentsGroup.m_vectSegmentCollisionComp.emplace_back(SegmentCollisionComponent());
                break;
            case Components_e::RECTANGLE_COLLISION_COMPONENT:
                m_refComponents[numEntity][Components_e::RECTANGLE_COLLISION_COMPONENT] = m_componentsGroup.m_vectRectangleCollisionComp.size();
                m_componentsGroup.m_vectRectangleCollisionComp.emplace_back(RectangleCollisionComponent());
                break;
            case Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT:
                m_refComponents[numEntity][Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = m_componentsGroup.m_vectFPSVisibleStaticElementComp.size();
                m_componentsGroup.m_vectFPSVisibleStaticElementComp.emplace_back(FPSVisibleStaticElementComponent());
                break;
            case Components_e::GENERAL_COLLISION_COMPONENT:
                m_refComponents[numEntity][Components_e::GENERAL_COLLISION_COMPONENT] = m_componentsGroup.m_vectGeneralCollisionComp.size();
                m_componentsGroup.m_vectGeneralCollisionComp.emplace_back(GeneralCollisionComponent());
                break;
            case Components_e::MOVEABLE_COMPONENT:
                m_refComponents[numEntity][Components_e::MOVEABLE_COMPONENT] = m_componentsGroup.m_vectMoveableComp.size();
                m_componentsGroup.m_vectMoveableComp.emplace_back(MoveableComponent());
                break;
            case Components_e::VISION_COMPONENT:
                m_refComponents[numEntity][Components_e::VISION_COMPONENT] = m_componentsGroup.m_vectVisionComp.size();
                m_componentsGroup.m_vectVisionComp.emplace_back(VisionComponent());
                break;
            case Components_e::TIMER_COMPONENT:
                m_refComponents[numEntity][Components_e::TIMER_COMPONENT] = m_componentsGroup.m_vectTimerComp.size();
                m_componentsGroup.m_vectTimerComp.emplace_back(TimerComponent());
                break;
            case Components_e::DOOR_COMPONENT:
                m_refComponents[numEntity][Components_e::DOOR_COMPONENT] = m_componentsGroup.m_vectDoorComp.size();
                m_componentsGroup.m_vectDoorComp.emplace_back(DoorComponent());
                break;
            case Components_e::PLAYER_CONF_COMPONENT:
                m_refComponents[numEntity][Components_e::PLAYER_CONF_COMPONENT] = m_componentsGroup.m_vectPlayerConfComp.size();
                m_componentsGroup.m_vectPlayerConfComp.emplace_back(PlayerConfComponent());
                break;
            case Components_e::ENEMY_CONF_COMPONENT:
                m_refComponents[numEntity][Components_e::ENEMY_CONF_COMPONENT] = m_componentsGroup.m_vectEnemyConfComp.size();
                m_componentsGroup.m_vectEnemyConfComp.emplace_back(EnemyConfComponent());
                break;
            case Components_e::MEM_POSITIONS_VERTEX_COMPONENT:
                m_refComponents[numEntity][Components_e::MEM_POSITIONS_VERTEX_COMPONENT] = m_componentsGroup.m_vectMemPositionsVertexComp.size();
                m_componentsGroup.m_vectMemPositionsVertexComp.emplace_back(MemPositionsVertexComponents());
                break;
            case Components_e::WRITE_COMPONENT:
                m_refComponents[numEntity][Components_e::WRITE_COMPONENT] = m_componentsGroup.m_vectWriteComp.size();
                m_componentsGroup.m_vectWriteComp.emplace_back(WriteComponent());
                break;
            case Components_e::SHOT_CONF_COMPONENT:
                m_refComponents[numEntity][Components_e::SHOT_CONF_COMPONENT] = m_componentsGroup.m_vectShotConfComp.size();
                m_componentsGroup.m_vectShotConfComp.emplace_back(ShotConfComponent());
                break;
            case Components_e::OBJECT_CONF_COMPONENT:
                m_refComponents[numEntity][Components_e::OBJECT_CONF_COMPONENT] = m_componentsGroup.m_vectObjectConfComp.size();
                m_componentsGroup.m_vectObjectConfComp.emplace_back(ObjectConfComponent());
                break;
            case Components_e::IMPACT_CONF_COMPONENT:
                m_refComponents[numEntity][Components_e::IMPACT_CONF_COMPONENT] = m_componentsGroup.m_vectImpactShotComp.size();
                m_componentsGroup.m_vectImpactShotComp.emplace_back(ImpactShotComponent());
                break;
            case Components_e::WEAPON_COMPONENT:
                m_refComponents[numEntity][Components_e::WEAPON_COMPONENT] = m_componentsGroup.m_vectWeaponComp.size();
                m_componentsGroup.m_vectWeaponComp.emplace_back(WeaponComponent());
                break;
            case Components_e::MOVEABLE_WALL_CONF_COMPONENT:
                m_refComponents[numEntity][Components_e::MOVEABLE_WALL_CONF_COMPONENT] = m_componentsGroup.m_vectMoveableWallConfComp.size();
                m_componentsGroup.m_vectMoveableWallConfComp.emplace_back(MoveableWallConfComponent());
                break;
            case Components_e::TRIGGER_COMPONENT:
                m_refComponents[numEntity][Components_e::TRIGGER_COMPONENT] = m_componentsGroup.m_vectTriggerComp.size();
                m_componentsGroup.m_vectTriggerComp.emplace_back(TriggerComponent());
                break;
            case Components_e::TELEPORT_COMPONENT:
                m_refComponents[numEntity][Components_e::TELEPORT_COMPONENT] = m_componentsGroup.m_vectTeleportComp.size();
                m_componentsGroup.m_vectTeleportComp.emplace_back(TeleportComponent());
                break;
            case Components_e::MEM_FPS_GLSIZE_COMPONENT:
                m_refComponents[numEntity][Components_e::MEM_FPS_GLSIZE_COMPONENT] = m_componentsGroup.m_vectMemFPSGLSizeComp.size();
                m_componentsGroup.m_vectMemFPSGLSizeComp.emplace_back(MemFPSGLSizeComponent());
                break;
            case Components_e::BARREL_COMPONENT:
                m_refComponents[numEntity][Components_e::BARREL_COMPONENT] = m_componentsGroup.m_vectBarrelComp.size();
                m_componentsGroup.m_vectBarrelComp.emplace_back(BarrelComponent());
                break;
            case Components_e::AUDIO_COMPONENT:
                m_refComponents[numEntity][Components_e::AUDIO_COMPONENT] = m_componentsGroup.m_vectAudioComp.size();
                m_componentsGroup.m_vectAudioComp.emplace_back(AudioComponent());
                break;
            case Components_e::CHECKPOINT_COMPONENT:
                m_refComponents[numEntity][Components_e::CHECKPOINT_COMPONENT] = m_componentsGroup.m_vectCheckpointComp.size();
                m_componentsGroup.m_vectCheckpointComp.emplace_back(CheckpointComponent());
                break;
            case Components_e::WALL_MULTI_SPRITE_CONF:
                m_refComponents[numEntity][Components_e::WALL_MULTI_SPRITE_CONF] = m_componentsGroup.m_vectWallMultiSpriteConfComp.size();
                m_componentsGroup.m_vectWallMultiSpriteConfComp.emplace_back(WallMultiSpriteConf());
                break;
            case Components_e::LOG_COMPONENT:
                m_refComponents[numEntity][Components_e::LOG_COMPONENT] = m_componentsGroup.m_vectLogComp.size();
                m_componentsGroup.m_vectLogComp.emplace_back(LogComponent());
                break;
            }
        }
    }
}

//===================================================================
void NewComponentManager::clear()
{
    m_componentsGroup.m_vectAudioComp.clear();
    m_componentsGroup.m_vectBarrelComp.clear();
    m_componentsGroup.m_vectCheckpointComp.clear();
    m_componentsGroup.m_vectCircleCollisionComp.clear();
    m_componentsGroup.m_vectColorVertexComp.clear();
    m_componentsGroup.m_vectDoorComp.clear();
    m_componentsGroup.m_vectEnemyConfComp.clear();
    m_componentsGroup.m_vectFPSVisibleStaticElementComp.clear();
    m_componentsGroup.m_vectGeneralCollisionComp.clear();
    m_componentsGroup.m_vectImpactShotComp.clear();
    m_componentsGroup.m_vectLogComp.clear();
    m_componentsGroup.m_vectMapCoordComp.clear();
    m_componentsGroup.m_vectMemFPSGLSizeComp.clear();
    m_componentsGroup.m_vectMemPositionsVertexComp.clear();
    m_componentsGroup.m_vectMemSpriteDataComp.clear();
    m_componentsGroup.m_vectMoveableComp.clear();
    m_componentsGroup.m_vectMoveableWallConfComp.clear();
    m_componentsGroup.m_vectObjectConfComp.clear();
    m_componentsGroup.m_vectPositionVertexComp.clear();
    m_componentsGroup.m_vectRectangleCollisionComp.clear();
    m_componentsGroup.m_vectSegmentCollisionComp.clear();
    m_componentsGroup.m_vectShotConfComp.clear();
    m_componentsGroup.m_vectSpriteTextureComp.clear();
    m_componentsGroup.m_vectTeleportComp.clear();
    m_componentsGroup.m_vectTimerComp.clear();
    m_componentsGroup.m_vectTriggerComp.clear();
    m_componentsGroup.m_vectVisionComp.clear();
    m_componentsGroup.m_vectWallMultiSpriteConfComp.clear();
    m_componentsGroup.m_vectWeaponComp.clear();
    m_componentsGroup.m_vectWriteComp.clear();
    m_componentsGroup.m_vectInputComp.clear();
    m_componentsGroup.m_vectPlayerConfComp.clear();
    m_refComponents.clear();
}

//===================================================================
void NewComponentManager::rmEntityComponents(uint32_t entityNum, const std::vector<bool> &componentVect)
{
    for(uint32_t i = 0; i < componentVect.size(); ++i)
    {
        if(componentVect[i])
        {
            switch (i)
            {
            case Components_e::POSITION_VERTEX_COMPONENT:
                m_refComponents[entityNum][Components_e::POSITION_VERTEX_COMPONENT] = std::nullopt;
                break;
            case Components_e::SPRITE_TEXTURE_COMPONENT:
                m_refComponents[entityNum][Components_e::SPRITE_TEXTURE_COMPONENT] = std::nullopt;
                break;
            case Components_e::MEM_SPRITE_DATA_COMPONENT:
                m_refComponents[entityNum][Components_e::MEM_SPRITE_DATA_COMPONENT] = std::nullopt;
                break;
            case Components_e::COLOR_VERTEX_COMPONENT:
                m_refComponents[entityNum][Components_e::COLOR_VERTEX_COMPONENT] = std::nullopt;
                break;
            case Components_e::MAP_COORD_COMPONENT:
                m_refComponents[entityNum][Components_e::MAP_COORD_COMPONENT] = std::nullopt;
                break;
            case Components_e::INPUT_COMPONENT:
                m_refComponents[entityNum][Components_e::INPUT_COMPONENT] = std::nullopt;
                break;
            case Components_e::CIRCLE_COLLISION_COMPONENT:
                m_refComponents[entityNum][Components_e::CIRCLE_COLLISION_COMPONENT] = std::nullopt;
                break;
            case Components_e::SEGMENT_COLLISION_COMPONENT:
                m_refComponents[entityNum][Components_e::SEGMENT_COLLISION_COMPONENT] = std::nullopt;
                break;
            case Components_e::RECTANGLE_COLLISION_COMPONENT:
                m_refComponents[entityNum][Components_e::RECTANGLE_COLLISION_COMPONENT] = std::nullopt;
                break;
            case Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT:
                m_refComponents[entityNum][Components_e::FPS_VISIBLE_STATIC_ELEMENT_COMPONENT] = std::nullopt;
                break;
            case Components_e::GENERAL_COLLISION_COMPONENT:
                m_refComponents[entityNum][Components_e::GENERAL_COLLISION_COMPONENT] = std::nullopt;
                break;
            case Components_e::MOVEABLE_COMPONENT:
                m_refComponents[entityNum][Components_e::MOVEABLE_COMPONENT] = std::nullopt;
                break;
            case Components_e::VISION_COMPONENT:
                m_refComponents[entityNum][Components_e::VISION_COMPONENT] = std::nullopt;
                break;
            case Components_e::TIMER_COMPONENT:
                m_refComponents[entityNum][Components_e::TIMER_COMPONENT] = std::nullopt;
                break;
            case Components_e::DOOR_COMPONENT:
                m_refComponents[entityNum][Components_e::DOOR_COMPONENT] = std::nullopt;
                break;
            case Components_e::PLAYER_CONF_COMPONENT:
                m_refComponents[entityNum][Components_e::PLAYER_CONF_COMPONENT] = std::nullopt;
                break;
            case Components_e::ENEMY_CONF_COMPONENT:
                m_refComponents[entityNum][Components_e::ENEMY_CONF_COMPONENT] = std::nullopt;
                break;
            case Components_e::MEM_POSITIONS_VERTEX_COMPONENT:
                m_refComponents[entityNum][Components_e::MEM_POSITIONS_VERTEX_COMPONENT] = std::nullopt;
                break;
            case Components_e::WRITE_COMPONENT:
                m_refComponents[entityNum][Components_e::WRITE_COMPONENT] = std::nullopt;
                break;
            case Components_e::SHOT_CONF_COMPONENT:
                m_refComponents[entityNum][Components_e::SHOT_CONF_COMPONENT] = std::nullopt;
                break;
            case Components_e::OBJECT_CONF_COMPONENT:
                m_refComponents[entityNum][Components_e::OBJECT_CONF_COMPONENT] = std::nullopt;
                break;
            case Components_e::IMPACT_CONF_COMPONENT:
                m_refComponents[entityNum][Components_e::IMPACT_CONF_COMPONENT] = std::nullopt;
                break;
            case Components_e::WEAPON_COMPONENT:
                m_refComponents[entityNum][Components_e::WEAPON_COMPONENT] = std::nullopt;
                break;
            case Components_e::MOVEABLE_WALL_CONF_COMPONENT:
                m_refComponents[entityNum][Components_e::MOVEABLE_WALL_CONF_COMPONENT] = std::nullopt;
                break;
            case Components_e::TRIGGER_COMPONENT:
                m_refComponents[entityNum][Components_e::TRIGGER_COMPONENT] = std::nullopt;
                break;
            case Components_e::TELEPORT_COMPONENT:
                m_refComponents[entityNum][Components_e::TELEPORT_COMPONENT] = std::nullopt;
                break;
            case Components_e::MEM_FPS_GLSIZE_COMPONENT:
                m_refComponents[entityNum][Components_e::MEM_FPS_GLSIZE_COMPONENT] = std::nullopt;
                break;
            case Components_e::BARREL_COMPONENT:
                m_refComponents[entityNum][Components_e::BARREL_COMPONENT] = std::nullopt;
                break;
            case Components_e::AUDIO_COMPONENT:
                m_refComponents[entityNum][Components_e::AUDIO_COMPONENT] = std::nullopt;
                break;
            case Components_e::CHECKPOINT_COMPONENT:
                m_refComponents[entityNum][Components_e::CHECKPOINT_COMPONENT] = std::nullopt;
                break;
            case Components_e::WALL_MULTI_SPRITE_CONF:
                m_refComponents[entityNum][Components_e::WALL_MULTI_SPRITE_CONF] = std::nullopt;
                break;
            case Components_e::LOG_COMPONENT:
                m_refComponents[entityNum][Components_e::LOG_COMPONENT] = std::nullopt;
                break;
            }
        }
    }
}

//===================================================================
std::optional<uint32_t> NewComponentManager::getComponentEmplacement(uint32_t numEntity, Components_e compNum)
{
    if(numEntity >= m_refComponents.size() || !m_refComponents[numEntity][compNum])
    {
        return std::nullopt;
    }
    return m_refComponents[numEntity][compNum];
}
