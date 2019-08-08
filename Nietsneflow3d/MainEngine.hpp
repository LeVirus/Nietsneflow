#ifndef MAINENGINE_H
#define MAINENGINE_H

#include <GraphicEngine.hpp>
#include <ECS/ECSManager.hpp>

using pairUI_t = std::pair<uint32_t, uint32_t>;
using pairFloat_t = std::pair<float, float>;

class LevelManager;
class Level;

class MainEngine
{
private:
    GraphicEngine m_graphicEngine;
    ECSManager m_ecsManager;
private:
    void loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                      const GroundCeilingData &ceilingData);
    void confCeilingComponents(uint32_t entityNum);
    void confGroundComponents(uint32_t entityNum);
    void linkSystemsToGraphicEngine();
    void loadPlayerEntity(const Level &level);
    void confPlayerEntity(uint32_t entityNum, const Level &level);
    void loadWallEntities(const LevelManager &levelManager);
    void loadStaticElementEntities(const LevelManager &levelManager);
    uint32_t createWallEntity();
    void confBaseMapComponent(uint32_t entityNum, const SpriteData &memSpriteData, const pairUI_t &coordLevel);
    uint32_t createStaticEntity();
    void confStaticMapComponent(uint32_t entityNum,
                                const pairFloat_t &elementSize,
                                bool traversable,
                                LevelStaticElementType_e type);
public:
    MainEngine();
    void loadGraphicPicture(const PictureData &picData);
    void init();
    void loadLevelEntities(const LevelManager &levelManager);
    void launchLoop();
};

#endif // MAINENGINE_H
