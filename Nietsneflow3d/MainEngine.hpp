#ifndef MAINENGINE_H
#define MAINENGINE_H

#include <GraphicEngine.h>

class MainEngine
{
private:
    GraphicEngine m_graphicEngine;
public:
    MainEngine();
    void loadGraphicPicture(const PictureData &picData);
    void testGraphicEngine();
};

#endif // MAINENGINE_H
