#pragma once

#include <vector>
#include <stdint.h>
#include <constants.hpp>
#include <cassert>

struct PositionVertexComponent;
struct ColorVertexComponent;
struct SpriteTextureComponent;
struct DoorComponent;
struct RayCastingIntersect;
struct WriteComponent;

using mapRayCastingData_t = std::map<uint32_t, std::vector<RayCastingIntersect>>;
using pairRaycastingData_t = std::pair<uint32_t, std::vector<RayCastingIntersect>>;
using groundCeilingRaycastContainer_t = std::array<std::vector<pairFloat_t>, RAYCAST_LINE_NUMBER>;

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
    inline const std::vector<uint32_t> &getVectIndices()const{return m_indices;}
    bool loadVertexColorComponent(const PositionVertexComponent *posComp,
                                  const ColorVertexComponent *colorComp);
    void loadVertexStandartTextureComponent(const PositionVertexComponent &posComp,
                                            SpriteTextureComponent &spriteComp);
    void loadVertexWriteTextureComponent(const PositionVertexComponent &posComp,
                                         const WriteComponent &writeComp);
    float loadWallDoorRaycastingEntity(const SpriteTextureComponent &spriteComp,
                                       const std::vector<RayCastingIntersect> &raycastingData,
                                       uint32_t totalLateralLine);
    void loadGroundRaycastingEntity(const groundCeilingRaycastContainer_t &groundCeilingRaycastPoint,
                                    const SpriteTextureComponent *spriteComp,
                                    const pairFloat_t &observerPoint, float observerAngleRadiant);
    void loadVertexTextureDrawByLineComponent(const PositionVertexComponent &posComp,
                                              const SpriteTextureComponent &spriteComp,
                                              uint32_t lineDrawNumber, DoorComponent *doorComp = nullptr);
    void loadVertexTextureDrawByLineRect(const pairFloat_t &firstPos,
                                         const pairFloat_t &secondPos,
                                         const SpriteTextureComponent &spriteComp,
                                         uint32_t lineDrawNumber, DoorComponent *doorComp, bool doorPosBound);
private:
    void addIndices(BaseShapeTypeGL_e shapeType);
    void init();
    void genGLBuffers();
    void setVectGLPointer();
    void bindGLBuffers();
    void attribGLVertexPointer();
    void addTexturePoint(const pairFloat_t &pos, const pairFloat_t &tex);
private:
    std::vector<float> m_vertexBuffer;
    std::vector<uint32_t> m_indices, m_shaderInterpretData;
    Shader_e m_shaderNum;
    uint32_t m_cursor = 0, m_sizeOfVertex;
    uint32_t m_ebo, m_vao, m_vbo;
};

pairFloat_t getTextureCoord(const pairFloat_t &pointA, const pairFloat_t &pointB,
                            const std::array<pairFloat_t, 4> &texturePosVertex, const pairFloat_t &textureSize);
