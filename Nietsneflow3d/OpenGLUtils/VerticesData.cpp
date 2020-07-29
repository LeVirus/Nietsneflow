#include "VerticesData.hpp"
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <PictureData.hpp>
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
    case Shader_e::COLOR_S:
        m_shaderInterpretData = {2,3};
        break;
    case Shader_e::TEXTURE_S:
        m_shaderInterpretData = {2,2};
        break;
    case Shader_e::TOTAL_SHADER_S:
        assert("Incoherant shader enum.");
    }
    m_sizeOfVertex = std::accumulate(m_shaderInterpretData.begin(),
                                         m_shaderInterpretData.end(), 0);
}

//===================================================================
bool VerticesData::loadVertexColorComponent(const PositionVertexComponent *posComp,
                                            const ColorVertexComponent *colorComp)
{
    if(m_shaderNum != Shader_e::COLOR_S)
    {
        return false;
    }
    assert(posComp && "Position component is Null.");
    assert(colorComp && "Color component is Null.");
    size_t sizeVertex = posComp->m_vertex.size();
    for(uint32_t j = 0; j < sizeVertex; ++j)
    {
        m_vertexBuffer.emplace_back(posComp->m_vertex[j].first);
        m_vertexBuffer.emplace_back(posComp->m_vertex[j].second);
        m_vertexBuffer.emplace_back(std::get<0>(colorComp->m_vertex[j]));
        m_vertexBuffer.emplace_back(std::get<1>(colorComp->m_vertex[j]));
        m_vertexBuffer.emplace_back(std::get<2>(colorComp->m_vertex[j]));
    }
    BaseShapeTypeGL_e shapeType = (sizeVertex == 3 ? BaseShapeTypeGL_e::TRIANGLE :
                                             BaseShapeTypeGL_e::RECTANGLE);
    addIndices(shapeType);
    return true;
}

//===================================================================
void VerticesData::loadVertexTextureComponent(const PositionVertexComponent &posComp,
                                              SpriteTextureComponent &spriteComp)
{
    if(m_shaderNum != Shader_e::TEXTURE_S)
    {
        return;
    }
    size_t sizeVertex = posComp.m_vertex.size();
    bool reverseTexture = false;
    if(posComp.m_vertex[1].first < posComp.m_vertex[0].first)
    {
        reverseTexture = true;
    }
    uint32_t k;
    for(uint32_t j = 0; j < 4; ++j)
    {
        if(reverseTexture)
        {
            if(j % 2 == 0)
            {
                k = j + 1;
            }
            else
            {
                k = j - 1;
            }
        }
        else
        {
            k = j;
        }
        m_vertexBuffer.emplace_back(posComp.m_vertex[j].first);
        m_vertexBuffer.emplace_back(posComp.m_vertex[j].second);
        //CrASH!!!!!!!!!!!!!!!!
        if(spriteComp.m_limitPointActive)
        {
            assert(spriteComp.m_limitSpriteData);
            m_vertexBuffer.emplace_back(spriteComp.m_limitSpriteData->m_texturePosVertex[k].first);
            m_vertexBuffer.emplace_back(spriteComp.m_limitSpriteData->m_texturePosVertex[k].second);
        }
        else
        {
            m_vertexBuffer.emplace_back(spriteComp.m_spriteData->m_texturePosVertex[k].first);
            m_vertexBuffer.emplace_back(spriteComp.m_spriteData->m_texturePosVertex[k].second);
        }
    }
    if(sizeVertex > 4)
    {
        uint32_t k, l;
        for(uint32_t j = 0; j < 4; ++j)
        {
            if(j == 0)
            {
                k = 1;
            }
            else if(j == 1)
            {
                k = 4;
            }
            else if(j == 2)
            {
                k = 5;
            }
            else
            {
                k = 2;
            }

            if(reverseTexture)
            {
                if(j % 2 == 0)
                {
                    l = j + 1;
                }
                else
                {
                    l = j - 1;
                }
            }
            else
            {
                l = j;
            }
            m_vertexBuffer.emplace_back(posComp.m_vertex[k].first);
            m_vertexBuffer.emplace_back(posComp.m_vertex[k].second);
            //CrASH!!!!!!!!!!!!!!!!OOOOOOOOOOOOOOOK
            if(spriteComp.m_limitPointActive)
            {
                m_vertexBuffer.emplace_back(spriteComp.m_limitSpriteData->m_texturePosVertex[l].first);
                m_vertexBuffer.emplace_back(spriteComp.m_limitSpriteData->m_texturePosVertex[l].second);
                //reset behaviour
                spriteComp.m_limitPointActive = false;
            }
            else
            {
                m_vertexBuffer.emplace_back(spriteComp.m_spriteData->m_texturePosVertex[l].first);
                m_vertexBuffer.emplace_back(spriteComp.m_spriteData->m_texturePosVertex[l].second);
            }
        }
    }
    if(sizeVertex == 3)
    {
        addIndices(BaseShapeTypeGL_e::TRIANGLE);
    }
    if(sizeVertex == 4)
    {
        addIndices(BaseShapeTypeGL_e::RECTANGLE);
    }
    else
    {
        addIndices(BaseShapeTypeGL_e::DOUBLE_RECT);
    }
}

//===================================================================
void VerticesData::addIndices(BaseShapeTypeGL_e shapeType)
{
    if(shapeType == BaseShapeTypeGL_e::NONE)
    {
        assert(false);
        return;
    }
    uint32_t curent = m_cursor;
    //first triangle
    m_indices.emplace_back(curent);//0
    m_indices.emplace_back(++curent);//1
    m_indices.emplace_back(++curent);//2
    //if Triangle stop here
    if(shapeType != BaseShapeTypeGL_e::TRIANGLE)
    {
        m_indices.emplace_back(curent);//2
        m_indices.emplace_back(++curent);//3
        m_indices.emplace_back(curent - 3);//0
    }
    if(shapeType == BaseShapeTypeGL_e::DOUBLE_RECT)
    {
        m_indices.emplace_back(++curent);//1
        m_indices.emplace_back(++curent);//4
        m_indices.emplace_back(++curent);//5
        m_indices.emplace_back(curent);//5
        m_indices.emplace_back(++curent);//2
        m_indices.emplace_back(curent - 3);//1
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
    uint64_t offset = 0;
    for(uint32_t i = 0; i < m_shaderInterpretData.size(); ++i)
    {
        glVertexAttribPointer(i, m_shaderInterpretData[i], GL_FLOAT, GL_FALSE,
                              m_sizeOfVertex * sizeof(float), (void*)(offset));
        offset = m_shaderInterpretData[i] * sizeof(float);
        glEnableVertexAttribArray(i);
    }
}

//===================================================================
void VerticesData::drawElement()
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

//===================================================================
void VerticesData::clear()
{
    m_vertexBuffer.clear();
    m_indices.clear();
    m_cursor = 0;
}
