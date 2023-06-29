#pragma once

#include <vector>
#include <ECS/Components/AudioComponent.hpp>
#include <ECS/Components/BarrelComponent.hpp>
#include <ECS/Components/CheckpointComponent.hpp>
#include <ECS/Components/CircleCollisionComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/EnemyConfComponent.hpp>
#include <ECS/Components/FPSVisibleStaticElementComponent.hpp>
#include <ECS/Components/GeneralCollisionComponent.hpp>
#include <ECS/Components/ImpactShotComponent.hpp>
#include <ECS/Components/InputComponent.hpp>
#include <ECS/Components/LogComponent.hpp>
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/MemFPSGLSizeComponent.hpp>
#include <ECS/Components/MemPositionsVertexComponents.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <ECS/Components/MoveableWallConfComponent.hpp>
#include <ECS/Components/ObjectConfComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/RectangleCollisionComponent.hpp>
#include <ECS/Components/SegmentCollisionComponent.hpp>
#include <ECS/Components/ShotConfComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/TeleportComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/TriggerComponent.hpp>
#include <ECS/Components/VisionComponent.hpp>
#include <ECS/Components/WallMultiSpriteConf.hpp>
#include <ECS/Components/WeaponComponent.hpp>
#include <ECS/Components/WriteComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>

struct ComponentsGroup
{
    std::vector<AudioComponent> m_vectAudioComp;
    std::vector<BarrelComponent> m_vectBarrelComp;
    std::vector<CheckpointComponent> m_vectCheckpointComp;
    std::vector<CircleCollisionComponent> m_vectCircleCollisionComp;
    std::vector<ColorVertexComponent> m_vectColorVertexComp;
    std::vector<DoorComponent> m_vectDoorComp;
    std::vector<EnemyConfComponent> m_vectEnemyConfComp;
    std::vector<FPSVisibleStaticElementComponent> m_vectFPSVisibleStaticElementComp;
    std::vector<GeneralCollisionComponent> m_vectGeneralCollisionComp;
    std::vector<ImpactShotComponent> m_vectImpactShotComp;
    std::vector<LogComponent> m_vectLogComp;
    std::vector<MapCoordComponent> m_vectMapCoordComp;
    std::vector<MemFPSGLSizeComponent> m_vectMemFPSGLSizeComp;
    std::vector<MemPositionsVertexComponents> m_vectMemPositionsVertexComp;
    std::vector<MemSpriteDataComponent> m_vectMemSpriteDataComp;
    std::vector<MoveableComponent> m_vectMoveableComp;
    std::vector<MoveableWallConfComponent> m_vectMoveableWallConfComp;
    std::vector<ObjectConfComponent> m_vectObjectConfComp;
    std::vector<PositionVertexComponent> m_vectPositionVertexComp;
    std::vector<RectangleCollisionComponent> m_vectRectangleCollisionComp;
    std::vector<SegmentCollisionComponent> m_vectSegmentCollisionComp;
    std::vector<ShotConfComponent> m_vectShotConfComp;
    std::vector<SpriteTextureComponent> m_vectSpriteTextureComp;
    std::vector<TeleportComponent> m_vectTeleportComp;
    std::vector<TimerComponent> m_vectTimerComp;
    std::vector<TriggerComponent> m_vectTriggerComp;
    std::vector<VisionComponent> m_vectVisionComp;
    std::vector<WallMultiSpriteConf> m_vectWallMultiSpriteConfComp;
    std::vector<WeaponComponent> m_vectWeaponComp;
    std::vector<WriteComponent> m_vectWriteComp;
    std::vector<InputComponent> m_vectInputComp;
    std::vector<PlayerConfComponent> m_vectPlayerConfComp;
};
