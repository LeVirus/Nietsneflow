#include "VerticesData.hpp"
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <numeric>

//===================================================================
VerticesData::VerticesData(Shader_e shaderNum) : m_shaderNum(shaderNum)
{
    init();
}

//===================================================================
void VerticesData::confVertexBuffer()
{
    bindGLBuffers();
    attribGLVertexPointer();
}

//===================================================================
void VerticesData::init()
{
    genGLBuffers();
    setVectGLPointer();
}

//===================================================================
void VerticesData::setVectGLPointer()
{
    switch (m_shaderNum)
    {
    case Shader_e::CEILING_FLOOR:
        m_shaderInterpretData = {2,3};
        break;
    case Shader_e::WALL:
//        m_shaderInterpretData = {2,3};
        break;
    case Shader_e::STATIC_ELEMENT:
//        m_shaderInterpretData = {2,3};
        break;
    case Shader_e::TOTAL_SHADER:
        assert("Incoherant shader enum.");
    }
    m_sizeOfVertex = std::accumulate(m_shaderInterpretData.begin(),
                                         m_shaderInterpretData.end(), 0);
}

//===================================================================
void VerticesData::addIndices(BaseShapeType_e shapeType)
{
    uint32_t curent = m_cursor;
    //first triangle
    m_indices.emplace_back(curent);
    m_indices.emplace_back(++curent);
    m_indices.emplace_back(++curent);
    //if Triangle stop here
    if(shapeType == BaseShapeType_e::RECTANGLE)
    {
        m_indices.emplace_back(curent);
        m_indices.emplace_back(++curent);
        m_indices.emplace_back(curent - 3);
    }
    m_cursor = ++curent;
}

//===================================================================
void VerticesData::genGLBuffers()
{
    glGenBuffers(1, &m_ebo);
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
}

//===================================================================
void VerticesData::bindGLBuffers()
{
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertexBuffer.size(),
                 &m_vertexBuffer[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_indices.size(),
                 &m_indices[0], GL_STATIC_DRAW);
}

//===================================================================
void VerticesData::attribGLVertexPointer()
{
    uint32_t offset = 0;
    for(uint32_t i = 0; i < m_shaderInterpretData.size(); ++i)
    {
        glVertexAttribPointer(i, m_shaderInterpretData[i], GL_FLOAT, GL_FALSE,
                              m_sizeOfVertex * sizeof(float), (void*)offset);
        offset = m_shaderInterpretData[i] * sizeof(float);
        glEnableVertexAttribArray(i);
    }
}

//===================================================================
void VerticesData::drawElement()
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

//===================================================================
void VerticesData::clear()
{
    m_vertexBuffer.clear();
    m_indices.clear();
    m_cursor = 0;
}
