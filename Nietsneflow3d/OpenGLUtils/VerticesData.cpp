#include "VerticesData.hpp"
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/DoorComponent.hpp>
#include <ECS/Components/WriteComponent.hpp>
#include <ECS/Systems/FirstPersonDisplaySystem.hpp>
#include <PictureData.hpp>
#include <CollisionUtils.hpp>
#include <numeric>
#include <cassert>

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
        //first point, second color or texture
        m_shaderInterpretData = {2, 4};
        break;
    case Shader_e::TEXTURE_S:
        m_shaderInterpretData = {2, 2};
        break;
    case Shader_e::COLORED_TEXTURE_S:
        //2 first ==> pos, 4 ==> color, 2 text
        m_shaderInterpretData = {2, 4, 2};
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
    assert(posComp->m_vertex.size() == colorComp->m_vertex.size() && "Color and Pos Component does not match.");
    size_t sizeVertex = posComp->m_vertex.size();
    for(uint32_t j = 0; j < sizeVertex; ++j)
    {
        m_vertexBuffer.emplace_back(posComp->m_vertex[j].first);
        m_vertexBuffer.emplace_back(posComp->m_vertex[j].second);
        m_vertexBuffer.emplace_back(std::get<0>(colorComp->m_vertex[j]));
        m_vertexBuffer.emplace_back(std::get<1>(colorComp->m_vertex[j]));
        m_vertexBuffer.emplace_back(std::get<2>(colorComp->m_vertex[j]));
        m_vertexBuffer.emplace_back(std::get<3>(colorComp->m_vertex[j]));
    }
    if(sizeVertex > 4)
    {
        assert(sizeVertex % 4 == 0);
        uint32_t rectNumber = sizeVertex / 4;
        for(uint32_t j = 0; j < rectNumber; ++j)
        {
            addIndices(BaseShapeTypeGL_e::RECTANGLE);
        }
        return true;
    }
    BaseShapeTypeGL_e shapeType = (sizeVertex == 3 ? BaseShapeTypeGL_e::TRIANGLE : BaseShapeTypeGL_e::RECTANGLE);
    addIndices(shapeType);
    return true;
}

//===================================================================
void VerticesData::loadVertexStandartTextureComponent(const PositionVertexComponent &posComp, SpriteTextureComponent &spriteComp)
{
    size_t sizeVertex = posComp.m_vertex.size();
    //first rect 0  1   2   3
    for(uint32_t j = 0; j < 4; ++j)
    {
        addTexturePoint(posComp.m_vertex[j], spriteComp.m_spriteData->m_texturePosVertex[j]);
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
            addTexturePoint(posComp.m_vertex[k], spriteComp.m_spriteData->m_texturePosVertex[j]);
        }
    }
    BaseShapeTypeGL_e shape = BaseShapeTypeGL_e::RECTANGLE;
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
void VerticesData::loadVertexStandardEntityByLine(const PositionVertexComponent &posComp,
                                                  SpriteTextureComponent &spriteComp, float entityDistance,
                                                  const std::array<float, RAYCAST_LINE_NUMBER> &memRaycastDist)
{
    float lateralGLPosA, lateralGLPosB, lateralText;
    int32_t currentLine = (((posComp.m_vertex[0].first + 1.0f) * RAYCAST_LINE_NUMBER) / 2.0f),
            finalLine = (((posComp.m_vertex[1].first + 1.0f) * RAYCAST_LINE_NUMBER) / 2.0f),
            totalLine = finalLine - currentLine;
    lateralGLPosA = currentLine * m_raycastStep - 1.0f;
    lateralGLPosB = lateralGLPosA + m_raycastStep;
    float diffTotalTexturePos = (spriteComp.m_spriteData->m_texturePosVertex[1].first -
                                 spriteComp.m_spriteData->m_texturePosVertex[0].first);
    for(int32_t i = 0; i < totalLine; ++i, ++currentLine, lateralGLPosA += m_raycastStep, lateralGLPosB += m_raycastStep)
    {
        if(currentLine > static_cast<int32_t>(RAYCAST_LINE_NUMBER))
        {
            break;
        }
        if(currentLine < 0 || (memRaycastDist[currentLine] > 15.0f &&
                               entityDistance > memRaycastDist[currentLine]))
        {
            continue;
        }
        lateralText = spriteComp.m_spriteData->m_texturePosVertex[0].first +
                static_cast<float>(i) / static_cast<float>(totalLine) * diffTotalTexturePos;
        assert(spriteComp.m_reverseVisibilityRate);
        addColoredTexturePoint({lateralGLPosA, posComp.m_vertex[0].second},
        {lateralText, spriteComp.m_spriteData->m_texturePosVertex[0].second},
        {*spriteComp.m_reverseVisibilityRate, *spriteComp.m_reverseVisibilityRate, *spriteComp.m_reverseVisibilityRate, 1.0f});
        addColoredTexturePoint({lateralGLPosB, posComp.m_vertex[0].second},
        {lateralText, spriteComp.m_spriteData->m_texturePosVertex[1].second},
        {*spriteComp.m_reverseVisibilityRate, *spriteComp.m_reverseVisibilityRate, *spriteComp.m_reverseVisibilityRate, 1.0f});
        addColoredTexturePoint({lateralGLPosB, posComp.m_vertex[2].second},
        {lateralText, spriteComp.m_spriteData->m_texturePosVertex[2].second},
        {*spriteComp.m_reverseVisibilityRate, *spriteComp.m_reverseVisibilityRate, *spriteComp.m_reverseVisibilityRate, 1.0f});
        addColoredTexturePoint({lateralGLPosA, posComp.m_vertex[2].second},
        {lateralText, spriteComp.m_spriteData->m_texturePosVertex[3].second},
        {*spriteComp.m_reverseVisibilityRate, *spriteComp.m_reverseVisibilityRate, *spriteComp.m_reverseVisibilityRate, 1.0f});
        addIndices(BaseShapeTypeGL_e::RECTANGLE);
    }
}

//===================================================================
float VerticesData::loadRaycastingEntity(const SpriteTextureComponent &spriteComp, const std::vector<RayCastingIntersect> &raycastingData)
{
    float lateralGLPosA, lateralGLPosB, verticalPos, lateralText, distantDist = raycastingData[0].m_distance;
    float diffTotalTexturePos = (spriteComp.m_spriteData->m_texturePosVertex[1].first - spriteComp.m_spriteData->m_texturePosVertex[0].first);
    float fogIntensity;
    for(uint32_t i = 0; i < raycastingData.size(); ++i)
    {
        if(i > 0 && raycastingData[i].m_lateral == raycastingData[i - 1].m_lateral - 1)
        {
            lateralGLPosA += m_raycastStep;
            lateralGLPosB += m_raycastStep;
        }
        else
        {
            lateralGLPosA = raycastingData[i].m_lateral * m_raycastStep - 1.0f;
            lateralGLPosB = lateralGLPosA + m_raycastStep;
        }
        verticalPos = RAYCAST_VERTICAL_SIZE / (raycastingData[i].m_distance / LEVEL_TILE_SIZE_PX);
        lateralText = spriteComp.m_spriteData->m_texturePosVertex[0].first +
                (raycastingData[i].m_texturePos / LEVEL_TILE_SIZE_PX) * diffTotalTexturePos;
        fogIntensity = getFogIntensity(raycastingData[i].m_distance + LEVEL_TILE_SIZE_PX);
        addColoredTexturePoint({lateralGLPosA, verticalPos},
        {lateralText, spriteComp.m_spriteData->m_texturePosVertex[0].second},
        {fogIntensity, fogIntensity, fogIntensity, 1.0f});
        addColoredTexturePoint({lateralGLPosB, verticalPos},
        {lateralText, spriteComp.m_spriteData->m_texturePosVertex[1].second},
        {fogIntensity, fogIntensity, fogIntensity, 1.0f});
        addColoredTexturePoint({lateralGLPosB, -verticalPos},
        {lateralText, spriteComp.m_spriteData->m_texturePosVertex[2].second},
        {fogIntensity, fogIntensity, fogIntensity, 1.0f});
        addColoredTexturePoint({lateralGLPosA, -verticalPos},
        {lateralText, spriteComp.m_spriteData->m_texturePosVertex[3].second},
        {fogIntensity, fogIntensity, fogIntensity, 1.0f});
        addIndices(BaseShapeTypeGL_e::RECTANGLE);
        if(raycastingData[i].m_distance > distantDist)
        {
            distantDist = raycastingData[i].m_distance;
        }
    }
    return distantDist;
}

//===================================================================
void VerticesData::loadVertexWriteTextureComponent(const PositionVertexComponent &posComp, const WriteComponent &writeComp)
{
    uint32_t size = writeComp.m_fontSpriteData.size();
    uint32_t l = 0;
    for(uint32_t i = 0; i < size; ++i)
    {
        for(uint32_t k = 0; k < writeComp.m_fontSpriteData[i].size(); ++k, ++l)
        {
            for(uint32_t j = 0; j < 4; ++j)
            {
                addTexturePoint(posComp.m_vertex[l * 4 + j],
                        writeComp.m_fontSpriteData[i][k].get().m_texturePosVertex[j]);
            }
            addIndices(BaseShapeTypeGL_e::RECTANGLE);
        }
    }
}

//===================================================================
void VerticesData::loadPointBackgroundRaycasting(const SpriteTextureComponent *spriteComp,
                                                 const PairFloat_t &GLPosUpLeft,
                                                 const PairFloat_t &GLPosDownRight,
                                                 const PairFloat_t &textureSize,
                                                 const PairFloat_t &pairMod)
{
    PairFloat_t texturePoint = getPointTextureCoord(spriteComp->m_spriteData->m_texturePosVertex,
                                                    textureSize, pairMod);
    m_vertexBuffer.insert(m_vertexBuffer.end(), {GLPosUpLeft.first, GLPosDownRight.second,
                                                 0.5f, 0.5f, 0.5f, 1.0f,
                                                 texturePoint.first, texturePoint.second,
                                                 GLPosDownRight.first, GLPosDownRight.second,
                                                 0.5f, 0.5f, 0.5f, 1.0f,
                                                 texturePoint.first, texturePoint.second,
                                                 GLPosDownRight.first, GLPosUpLeft.second,
                                                 0.5f, 0.5f, 0.5f, 1.0f,
                                                 texturePoint.first, texturePoint.second,
                                                 GLPosUpLeft.first, GLPosUpLeft.second,
                                                 0.5f, 0.5f, 0.5f, 1.0f,
                                                 texturePoint.first, texturePoint.second});
    addIndices(BaseShapeTypeGL_e::RECTANGLE);
}

//===================================================================
void VerticesData::loadVertexTextureDrawByLineComponent(const PositionVertexComponent &posComp,
                                                        const SpriteTextureComponent &spriteComp,
                                                        uint32_t lineDrawNumber, DoorComponent *doorComp)
{
    assert(posComp.m_vertex.size() == 4 || posComp.m_vertex.size() == 6);
    std::pair<bool, bool> boundDoorTexture;
    if(doorComp)
    {
        boundDoorTexture.first = (doorComp->m_vertical == doorComp->m_verticalPosDisplay.first);
        boundDoorTexture.second = (doorComp->m_vertical == doorComp->m_verticalPosDisplay.second);
    }
    loadVertexTextureDrawByLineRect(posComp.m_vertex[0], posComp.m_vertex[1],
                                    spriteComp, lineDrawNumber, doorComp, boundDoorTexture.first);
    if(posComp.m_vertex.size() == 6)
    {
        loadVertexTextureDrawByLineRect(posComp.m_vertex[1], posComp.m_vertex[4],
                                        spriteComp, lineDrawNumber, doorComp, boundDoorTexture.second);
    }
}

//===================================================================
void VerticesData::loadVertexTextureDrawByLineRect(const PairFloat_t &firstPos,
                                                   const PairFloat_t &secondPos,
                                                   const SpriteTextureComponent &spriteComp,
                                                   uint32_t lineDrawNumber, DoorComponent *doorComp,
                                                   bool doorPosBound)
{
    float lineDrawNumberFloat = static_cast<float>(lineDrawNumber);
    PairFloat_t stepPos;
    stepPos.first = (secondPos.first - firstPos.first) / lineDrawNumberFloat;
    stepPos.second = (secondPos.second - firstPos.second) / lineDrawNumberFloat;
    float stepTex;
    PairFloat_t posLateralText;
    if(doorComp && doorPosBound)
    {
        posLateralText.first = spriteComp.m_spriteData->m_texturePosVertex[0].first;
        posLateralText.second = spriteComp.m_spriteData->m_texturePosVertex[0].first;
    }
    else
    {
        posLateralText = {spriteComp.m_spriteData->m_texturePosVertex[0].first,
                          spriteComp.m_spriteData->m_texturePosVertex[1].first};
    }

    PairFloat_t currentTexPos = {posLateralText.first, spriteComp.m_spriteData->m_texturePosVertex[0].second};
    stepTex = (posLateralText.second - posLateralText.first) /
            lineDrawNumberFloat;
    float memDownTexture = spriteComp.m_spriteData->m_texturePosVertex[2].second;
    PairFloat_t currentPos = firstPos, currentPreviousPos, currentPreviousTexPos;
    while(currentTexPos.first < posLateralText.second &&
          currentPos.first < 1.0f)
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

//===================================================================
void VerticesData::reserveVertex(uint32_t size)
{
    m_vertexBuffer.reserve(size);
}

//===================================================================
void VerticesData::reserveIndices(uint32_t size)
{
    m_indices.reserve(size);
}

//===================================================================
void VerticesData::addTexturePoint(const PairFloat_t &pos, const PairFloat_t &tex)
{
    m_vertexBuffer.insert(m_vertexBuffer.end(), {pos.first, pos.second, tex.first, tex.second});
}

//===================================================================
void VerticesData::addColoredTexturePoint(const PairFloat_t &pos, const PairFloat_t &tex, const std::array<float, 4> &color)
{
    m_vertexBuffer.insert(m_vertexBuffer.end(), {pos.first, pos.second, color[0], color[1], color[2], color[3], tex.first, tex.second});
}

//===================================================================
void VerticesData::addIndices(BaseShapeTypeGL_e shapeType)
{
    assert(shapeType != BaseShapeTypeGL_e::NONE);
    //first triangle
    if(shapeType == BaseShapeTypeGL_e::TRIANGLE)
    {
        m_indices.insert(m_indices.end(), {m_cursor, ++m_cursor, ++m_cursor});// 0 1 2
    }
    //if Triangle stop here
    else if(shapeType == BaseShapeTypeGL_e::RECTANGLE)
    {
        m_indices.insert(m_indices.end(), {m_cursor, ++m_cursor, ++m_cursor,
                                           m_cursor, ++m_cursor, m_cursor - 3});// 0 1 2 2 3 0
    }
    else if(shapeType == BaseShapeTypeGL_e::DOUBLE_RECT)
    {
        // 0 1 2 2 3 0  1 4 5 5 2 1
        m_indices.insert(m_indices.end(), {m_cursor, ++m_cursor, ++m_cursor,
                                           m_cursor, ++m_cursor, m_cursor - 3,
                                           ++m_cursor, ++m_cursor, ++m_cursor,
                                           m_cursor, ++m_cursor, m_cursor - 3});
    }
    ++m_cursor;
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
        offset += m_shaderInterpretData[i] * sizeof(float);
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
