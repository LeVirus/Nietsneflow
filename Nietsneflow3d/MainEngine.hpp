#ifndef MAINENGINE_H
#define MAINENGINE_H

#include <GraphicEngine.hpp>
#include <ECS/ECSManager.hpp>

class MainEngine
{
private:
    GraphicEngine m_graphicEngine;
    ECSManager m_ecsManager;
private:
    void loadGroundAndCeilingEntities(const GroundCeilingData &groundData,
                                      const GroundCeilingData &ceilingData);
    void linkSystemsToEngine();
public:
    MainEngine();
    void loadGraphicPicture(const PictureData &picData);
    void init();
    void loadECSEntities();
    void launchLoop();
};

#endif // MAINENGINE_H
