#include "VerticesData.hpp"

//===================================================================
VerticesData::VerticesData(Shader_e shaderNum) : m_shaderNum(shaderNum)
{
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
}

