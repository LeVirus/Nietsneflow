#include "VerticesData.hpp"

//===================================================================
VerticesData::VerticesData(Shader_e shaderNum) : m_shaderNum(shaderNum)
{
}

//===================================================================
void VerticesData::addIndices(BaseShapeType_t shapeType)
{
    uint32_t curent = m_cursor;
    //first triangle
    m_indices.emplace_back(curent);
    m_indices.emplace_back(++curent);
    m_indices.emplace_back(++curent);
    //if Triangle stop here
    if(shapeType == BaseShapeType_t::RECTANGLE)
    {
        m_indices.emplace_back(curent);
        m_indices.emplace_back(++curent);
        m_indices.emplace_back(curent - 3);
    }
    m_cursor = ++curent;
}

//===================================================================
void VerticesData::loadGLBuffer()
{
    if(m_vertexBuffer.empty() || m_indices.empty())
    {
        return;
    }
}

//===================================================================
void VerticesData::clear()
{
    m_vertexBuffer.clear();
    m_indices.clear();
    m_cursor = 0;
}

