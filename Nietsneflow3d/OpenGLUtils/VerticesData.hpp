#pragma once

#include <vector>
#include <functional>
#include <stdint.h>
#include <constants.hpp>
#include <cassert>

struct PositionVertexComponent;
struct ColorVertexComponent;
struct SpriteTextureComponent;
struct DoorComponent;
struct RayCastingIntersect;
struct WriteComponent;

using MapRayCastingData_t = std::map<uint32_t, std::vector<RayCastingIntersect>>;
using pairRaycastingData_t = std::pair<uint32_t, std::vector<RayCastingIntersect>>;

struct GroundCeililngRayCastingIntersect;
class Shader;

class VerticesData
{
public:
    VerticesData(Shader_e shaderNum = Shader_e::TEXTURE_S);
    void confVertexBuffer();
    void drawElement();
    void clear();
    inline void setShaderType(Shader_e shaderNum){m_shaderNum = shaderNum;}
    inline const std::vector<float> &getVectVertex()const{return m_vertexBuffer;}
    inline bool empty()const{return m_vertexBuffer.empty();}
    inline uint32_t getNumberOfIndex()const
    {
        return m_indices.size();
    }
    inline const std::vector<uint32_t> &getVectIndices()const{return m_indices;}
    bool loadVertexColorComponent(const PositionVertexComponent *posComp,
                                  const ColorVertexComponent *colorComp);
    void loadVertexStandartTextureComponent(const PositionVertexComponent &posComp,
                                            SpriteTextureComponent &spriteComp);
    void loadVertexStandardEntityByLine(const PositionVertexComponent &posComp,
                                            SpriteTextureComponent &spriteComp, float entityDistance, const std::array<float, RAYCAST_LINE_NUMBER> &memRaycastDist);
    void loadVertexWriteTextureComponent(const PositionVertexComponent &posComp,
                                         const WriteComponent &writeComp);
    float loadRaycastingEntity(const SpriteTextureComponent &spriteComp, const std::vector<RayCastingIntersect> &raycastingData);
    void loadPointBackgroundRaycasting(const SpriteTextureComponent *spriteComp,
                                       const PairFloat_t &GLPosUpLeft,
                                       const PairFloat_t &GLPosDownRight,
                                       const PairFloat_t &textureSize,
                                       const PairFloat_t &pairMod);
    void loadVertexTextureDrawByLineComponent(const PositionVertexComponent &posComp,
                                              const SpriteTextureComponent &spriteComp,
                                              uint32_t lineDrawNumber, DoorComponent *doorComp = nullptr);
    void loadVertexTextureDrawByLineRect(const PairFloat_t &firstPos,
                                         const PairFloat_t &secondPos,
                                         const SpriteTextureComponent &spriteComp,
                                         uint32_t lineDrawNumber, DoorComponent *doorComp, bool doorPosBound);
    void reserveVertex(uint32_t size);
    void reserveIndices(uint32_t size);
private:
    void addIndices(BaseShapeTypeGL_e shapeType);
    void init();
    void genGLBuffers();
    void setVectGLPointer();
    void bindGLBuffers();
    void attribGLVertexPointer();
    void addTexturePoint(const PairFloat_t &pos, const PairFloat_t &tex);
    void addColoredTexturePoint(const PairFloat_t &pos, const PairFloat_t &tex, const std::array<float, 4> &color);
private:
    std::vector<float> m_vertexBuffer;
    std::vector<uint32_t> m_indices, m_shaderInterpretData;
    Shader_e m_shaderNum;
    uint32_t m_cursor = 0, m_sizeOfVertex;
    uint32_t m_ebo, m_vao, m_vbo;
    float m_raycastStep = 2.0f / static_cast<float>(RAYCAST_LINE_NUMBER);
};

PairFloat_t getTexturePixelFromCoord(const PairFloat_t &pointA, const std::array<PairFloat_t, 4> &texturePosVertex);
//pairMod opti calc modulo before method call to prevent several calculation
inline PairFloat_t getPointTextureCoord(const std::array<PairFloat_t, 4> &texturePosVertex,
                                        const PairFloat_t &textureSize, const PairFloat_t &pairMod)
{
    return {texturePosVertex.at(0).first +
                (pairMod.first / LEVEL_TILE_SIZE_PX) * textureSize.first,
                texturePosVertex.at(0).second +
                (pairMod.second / LEVEL_TILE_SIZE_PX) * textureSize.second};
}
