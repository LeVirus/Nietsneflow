#pragma once

#include <GraphicEngine.hpp>
#include <PhysicalEngine.hpp>
#include <ECS/ECSManager.hpp>

class LevelManager;
class Level;

class MainEngine
{
private:
    GraphicEngine m_graphicEngine;
    PhysicalEngine m_physicalEngine;
    ECSManager m_ecsManager;
private:
    void loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                      const GroundCeilingData &ceilingData);
    void confCeilingComponents(uint32_t entityNum);
    void confGroundComponents(uint32_t entityNum);
    void linkSystemsToGraphicEngine();
    void linkSystemsToPhysicalEngine();
    void loadPlayerEntity(const Level &level);
    void confPlayerEntity(uint32_t entityNum, const Level &level);
    void loadWallEntities(const LevelManager &levelManager);
    void loadEnemiesEntities(const LevelManager &levelManager);
    void loadStaticElementEntities(const LevelManager &levelManager);
    uint32_t createMapWallEntity();
    uint32_t createMapEnemyEntity();
    void confBaseMapComponent(uint32_t entityNum, const SpriteData &memSpriteData, const pairUI_t &coordLevel, CollisionShape_e collisionShape);
    uint32_t createMapStaticEntity();
    void confStaticMapComponent(uint32_t entityNum,
                                const pairFloat_t &elementSize,
                                bool traversable,
                                LevelStaticElementType_e type);
public:
    MainEngine();
    void loadGraphicPicture(const PictureData &picData);
    void init();
    void loadLevelEntities(const LevelManager &levelManager);
    void mainLoop();
};
