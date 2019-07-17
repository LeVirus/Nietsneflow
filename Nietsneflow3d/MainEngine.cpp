#include "MainEngine.hpp"

//===================================================================
MainEngine::MainEngine()
{

}

//===================================================================
void MainEngine::loadGraphicPicture(const PictureData &picData)
{
    m_graphicEngine.loadPictureData(picData);
}

//TEST
void MainEngine::testGraphicEngine()
{
//    m_graphicEngine.initGLWindow();
//    m_graphicEngine.initGLShader();
}

