#ifndef MAINENGINE_H
#define MAINENGINE_H

#include <GraphicEngine.hpp>
#include <ECS/ECSManager.hpp>

using pairUI_t = std::pair<uint32_t, uint32_t>;

class LevelManager;

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
    void loadWallEntities(const LevelManager &levelManager);
    uint32_t createWallEntity();
    void confBaseMapComponent(uint32_t entityNum, const SpriteData &memSpriteData, const pairUI_t &coordLevel);
    void loadStaticElementEntities(const LevelManager &levelManager);

public:
    MainEngine();
    void loadGraphicPicture(const PictureData &picData);
    void init();
    void loadLevelEntities(const LevelManager &levelManager);
    void launchLoop();
};

#endif // MAINENGINE_H
