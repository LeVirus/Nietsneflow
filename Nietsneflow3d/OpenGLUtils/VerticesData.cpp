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
//    case Shader_e::TEXTURED_WALL_S:
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
void VerticesData::loadVertexStandartTextureComponent(const PositionVertexComponent &posComp,
                                              SpriteTextureComponent &spriteComp)
{
    size_t sizeVertex = posComp.m_vertex.size();
    //first rect 0  1   2   3
    for(uint32_t j = 0; j < 4; ++j)
    {
        //add target screen position to buffer
        m_vertexBuffer.emplace_back(posComp.m_vertex[j].first);
        m_vertexBuffer.emplace_back(posComp.m_vertex[j].second);
        //add texture position to buffer
        if(spriteComp.m_limitWallPointActive)
        {
            assert(spriteComp.m_limitWallSpriteData);
            m_vertexBuffer.emplace_back(spriteComp.m_limitWallSpriteData->at(j).first);
            m_vertexBuffer.emplace_back(spriteComp.m_limitWallSpriteData->at(j).second);
        }
        else
        {
            m_vertexBuffer.emplace_back(spriteComp.m_spriteData->m_texturePosVertex[j].first);
            m_vertexBuffer.emplace_back(spriteComp.m_spriteData->m_texturePosVertex[j].second);
        }
    }
    //treat second rect >> 1    4   5   2
    if(sizeVertex > 4)
    {
        uint32_t k;
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
            m_vertexBuffer.emplace_back(posComp.m_vertex[k].first);
            m_vertexBuffer.emplace_back(posComp.m_vertex[k].second);
            if(spriteComp.m_limitWallPointActive)
            {
                m_vertexBuffer.emplace_back(spriteComp.m_limitWallSpriteData->at(4 + j).first);
                m_vertexBuffer.emplace_back(spriteComp.m_limitWallSpriteData->at(4 + j).second);
                //reset behaviour
            }
            else
            {
                m_vertexBuffer.emplace_back(spriteComp.m_spriteData->m_texturePosVertex[j].first);
                m_vertexBuffer.emplace_back(spriteComp.m_spriteData->m_texturePosVertex[j].second);
            }
        }
    }
    spriteComp.m_limitWallPointActive = false;
    BaseShapeTypeGL_e shape;
    if(sizeVertex == 3)
    {
        shape = BaseShapeTypeGL_e::TRIANGLE;
    }
    else if(sizeVertex == 4)
    {
        shape = BaseShapeTypeGL_e::RECTANGLE;
    }
    else
    {
        shape = BaseShapeTypeGL_e::DOUBLE_RECT;
    }
    addIndices(shape);
}

//===================================================================
void VerticesData::loadVertexTextureDrawByLineComponent(const PositionVertexComponent &posComp,
                                                        const SpriteTextureComponent &spriteComp,
                                                        uint32_t lineDrawNumber)
{
    assert(posComp.m_vertex.size() == 4 || posComp.m_vertex.size() == 6);
    loadVertexTextureDrawByLineRect(posComp.m_vertex[0], posComp.m_vertex[1],
                                    spriteComp, lineDrawNumber);
    if(posComp.m_vertex.size() == 6)
    {
        loadVertexTextureDrawByLineRect(posComp.m_vertex[1], posComp.m_vertex[4],
                                        spriteComp, lineDrawNumber);
    }
}

//===================================================================
void VerticesData::loadVertexTextureDrawByLineRect(const pairFloat_t &firstPos,
                                                   const pairFloat_t &secondPos,
                                                   const SpriteTextureComponent &spriteComp,
                                                   uint32_t lineDrawNumber)
{
    if(!spriteComp.m_limitWallPointActive)
    {
        pairFloat_t stepPos;
        stepPos.first = (secondPos.first - firstPos.first) /
                static_cast<float>(lineDrawNumber);
        stepPos.second = (secondPos.second - firstPos.second) /
                static_cast<float>(lineDrawNumber);
        float stepTex = (spriteComp.m_spriteData->m_texturePosVertex[1].first -
                         spriteComp.m_spriteData->m_texturePosVertex[0].first) /
                static_cast<float>(lineDrawNumber);
        float memDownTexture = spriteComp.m_spriteData->m_texturePosVertex[2].second;
        pairFloat_t currentPos = firstPos, currentPreviousPos,
                currentTexPos = spriteComp.m_spriteData->m_texturePosVertex[0],
                currentPreviousTexPos;
        for(uint32_t i = 0; i < lineDrawNumber; ++i)
        {
            //up left
            addTexturePoint(currentPos, currentTexPos);
            currentPreviousPos = currentPos;
            currentPreviousTexPos = currentTexPos;
            currentPos.first += stepPos.first;
            currentPos.second += stepPos.second;
            currentTexPos.first += stepTex;
            //up right
            addTexturePoint(currentPos, currentTexPos);
            //down right
            addTexturePoint({currentPos.first, -currentPos.second},
            {currentTexPos.first, memDownTexture});
            //down left
            addTexturePoint({currentPreviousPos.first, -currentPreviousPos.second},
            {currentPreviousTexPos.first, memDownTexture});
            addIndices(BaseShapeTypeGL_e::RECTANGLE);
        }
    }
}

//===================================================================
void VerticesData::addTexturePoint(const pairFloat_t &pos, const pairFloat_t &tex)
{
    m_vertexBuffer.emplace_back(pos.first);
    m_vertexBuffer.emplace_back(pos.second);
    m_vertexBuffer.emplace_back(tex.first);
    m_vertexBuffer.emplace_back(tex.second);
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
