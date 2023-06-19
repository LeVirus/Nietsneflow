#pragma once

#include <ECS/ComponentsGroup.hpp>
#include <functional>
#include <optional>

using ArrayComp = std::array<std::optional<uint32_t>, Components_e::TOTAL_COMPONENTS>;

class NewComponentManager
{
public:
    NewComponentManager();
    void init();
    void addEntity(uint32_t numEntity, const std::bitset<Components_e::TOTAL_COMPONENTS> &bitsetComponents);
    void clear();
    inline ComponentsGroup &getComponentsContainer()
    {
        return m_componentsGroup;
    }
    inline std::vector<AudioComponent> &getVectAudioComp()
    {
        return m_componentsGroup.m_vectAudioComp;
    }
    inline std::vector<BarrelComponent> &getVectBarrelComp()
    {
        return m_componentsGroup.m_vectBarrelComp;
    }
    inline std::vector<CheckpointComponent> &getVectCheckpointComp()
    {
        return m_componentsGroup.m_vectCheckpointComp;
    }
    inline std::vector<CircleCollisionComponent> &getVectCircleCollComp()
    {
        return m_componentsGroup.m_vectCircleCollisionComp;
    }
    inline std::vector<ColorVertexComponent> &getVectColorVertexComp()
    {
        return m_componentsGroup.m_vectColorVertexComp;
    }
    inline std::vector<DoorComponent> &getVectDoorComp()
    {
        return m_componentsGroup.m_vectDoorComp;
    }
    inline std::vector<EnemyConfComponent> &getVectEnemyConfComp()
    {
        return m_componentsGroup.m_vectEnemyConfComp;
    }
    inline std::vector<FPSVisibleStaticElementComponent> &getVectFPSVisibleStaticElementComp()
    {
        return m_componentsGroup.m_vectFPSVisibleStaticElementComp;
    }
    inline std::vector<GeneralCollisionComponent> &getVectGeneralCollComp()
    {
        return m_componentsGroup.m_vectGeneralCollisionComp;
    }
    inline std::vector<ImpactShotComponent> &getVectImpactShotComp()
    {
        return m_componentsGroup.m_vectImpactShotComp;
    }
    inline std::vector<LogComponent> &getVectLogComp()
    {
        return m_componentsGroup.m_vectLogComp;
    }
    inline std::vector<MapCoordComponent> &getVectMapCoordComp()
    {
        return m_componentsGroup.m_vectMapCoordComp;
    }
    inline std::vector<MemFPSGLSizeComponent> &getVectMemFPSGLSizeComponent()
    {
        return m_componentsGroup.m_vectMemFPSGLSizeComp;
    }
    inline std::vector<MemPositionsVertexComponents> &getVectMemPositionsVertexComp()
    {
        return m_componentsGroup.m_vectMemPositionsVertexComp;
    }
    inline std::vector<MemSpriteDataComponent> &getVectMemSpriteDataComp()
    {
        return m_componentsGroup.m_vectMemSpriteDataComp;
    }
    inline std::vector<MoveableComponent> &getVectMoveableComp()
    {
        return m_componentsGroup.m_vectMoveableComp;
    }
    inline std::vector<MoveableWallConfComponent> &getVectMoveableWallConfComp()
    {
        return m_componentsGroup.m_vectMoveableWallConfComp;
    }
    inline std::vector<ObjectConfComponent> &getVectObjectConfComp()
    {
        return m_componentsGroup.m_vectObjectConfComp;
    }
    inline std::vector<PositionVertexComponent> &getVectPositionVertexComp()
    {
        return m_componentsGroup.m_vectPositionVertexComp;
    }
    inline std::vector<RectangleCollisionComponent> &getVectRectangleCollisionComp()
    {
        return m_componentsGroup.m_vectRectangleCollisionComp;
    }
    inline std::vector<SegmentCollisionComponent> &getVectSegmentCollisionComp()
    {
        return m_componentsGroup.m_vectSegmentCollisionComp;
    }
    inline std::vector<ShotConfComponent> &getVectShotConfComp()
    {
        return m_componentsGroup.m_vectShotConfComp;
    }
    inline std::vector<WriteComponent> &getVectWriteComp()
    {
        return m_componentsGroup.m_vectWriteComp;
    }
    inline std::vector<WeaponComponent> &getVectWeaponComp()
    {
        return m_componentsGroup.m_vectWeaponComp;
    }
    inline std::vector<WallMultiSpriteConf> &getVectWallMultiSpriteConfComp()
    {
        return m_componentsGroup.m_vectWallMultiSpriteConfComp;
    }
    inline std::vector<VisionComponent> &getVectVisionComp()
    {
        return m_componentsGroup.m_vectVisionComp;
    }
    inline std::vector<TriggerComponent> &getVectTriggerComp()
    {
        return m_componentsGroup.m_vectTriggerComp;
    }
    inline std::vector<TimerComponent> &getVectTimerComp()
    {
        return m_componentsGroup.m_vectTimerComp;
    }
    inline std::vector<TeleportComponent> &getVectTeleportComp()
    {
        return m_componentsGroup.m_vectTeleportComp;
    }
    inline std::vector<SpriteTextureComponent> &getVectSpriteTextureComp()
    {
        return m_componentsGroup.m_vectSpriteTextureComp;
    }
    inline std::vector<InputComponent> &getVectInputComp()
    {
        return m_componentsGroup.m_vectInputComp;
    }
    inline PlayerConfComponent &getPlayerConfComp()
    {
        return m_componentsGroup.m_playerConfComp;
    }
    inline const std::vector<ArrayComp> &getPlayerConfComp()const
    {
        return m_refComponents;
    }
    std::optional<uint32_t> getComponentEmplacement(uint32_t numEntity, Components_e compNum);
    PositionVertexComponent *getPosVertexComponent(uint32_t numEntity);
    SpriteTextureComponent *getSpriteTextureComponent(uint32_t numEntity);
    MemSpriteDataComponent *getMemSpriteDataComponent(uint32_t numEntity);
    ColorVertexComponent *getColorVertexComponent(uint32_t numEntity);
    MapCoordComponent *getMapCoordComponent(uint32_t numEntity);
    InputComponent *getInputComponent(uint32_t numEntity);
    CircleCollisionComponent *getCircleCollComponent(uint32_t numEntity);
    SegmentCollisionComponent *getSegmentCollisionComponent(uint32_t numEntity);
    RectangleCollisionComponent *getRectangleCollisionComponent(uint32_t numEntity);
    FPSVisibleStaticElementComponent *getFPSVisibleStaticElementComponent(uint32_t numEntity);
    GeneralCollisionComponent *getGeneralCollisionComponent(uint32_t numEntity);
    MoveableComponent *getMoveableComponent(uint32_t numEntity);
    VisionComponent *getVisionComponent(uint32_t numEntity);
    TimerComponent *getTimerComponent(uint32_t numEntity);
    DoorComponent *getDoorComponent(uint32_t numEntity);
    PlayerConfComponent *getPlayerConfComponent(uint32_t numEntity);
    EnemyConfComponent *getEnemyConfComponent(uint32_t numEntity);
    MemPositionsVertexComponents *getMemPositionsVertexComponent(uint32_t numEntity);
    WriteComponent *getWriteComponent(uint32_t numEntity);
    ShotConfComponent *getShotConfComponent(uint32_t numEntity);
    ObjectConfComponent *getObjectConfComponent(uint32_t numEntity);
    ImpactShotComponent *getImpactShotComponent(uint32_t numEntity);
    WeaponComponent *getWeaponComponent(uint32_t numEntity);
    MoveableWallConfComponent *getMoveableWallConfComponent(uint32_t numEntity);
    TriggerComponent *getTriggerComponent(uint32_t numEntity);
    TeleportComponent *getTeleportComponent(uint32_t numEntity);
    MemFPSGLSizeComponent *getMemFPSGLSizeComponent(uint32_t numEntity);
    BarrelComponent *getBarrelComponent(uint32_t numEntity);
    AudioComponent *getAudioComponent(uint32_t numEntity);
    CheckpointComponent *getCheckpointComponent(uint32_t numEntity);
    WallMultiSpriteConf *getWallMultiSpriteConfComponent(uint32_t numEntity);
    LogComponent *getLogComponent(uint32_t numEntity);
private:
    ComponentsGroup m_componentsGroup;
    //indicate which vector emplacement the component is
    std::vector<ArrayComp> m_refComponents;
};
