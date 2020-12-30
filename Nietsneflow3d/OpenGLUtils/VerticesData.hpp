#pragma once

#include <vector>
#include <stdint.h>
#include <constants.hpp>
#include <cassert>

struct PositionVertexComponent;
struct ColorVertexComponent;
struct SpriteTextureComponent;

class Shader;

class VerticesData
{
private:
    std::vector<float> m_vertexBuffer;
    std::vector<uint32_t> m_indices, m_shaderInterpretData;
    Shader_e m_shaderNum;
    uint32_t m_cursor = 0, m_sizeOfVertex;
    uint32_t m_ebo, m_vao, m_vbo;
private:
    void addIndices(BaseShapeTypeGL_e shapeType);
    void init();
    void genGLBuffers();
    void setVectGLPointer();
    void bindGLBuffers();
    void attribGLVertexPointer();
    void loadVertexStandartTextureComponent(const PositionVertexComponent &posComp,
                                            SpriteTextureComponent &spriteComp);
    void loadVertexTexturedWallComponent(const PositionVertexComponent &posComp,
                                         const SpriteTextureComponent &spriteComp);
public:
    VerticesData(Shader_e shaderNum);
    void confVertexBuffer();
    void drawElement();
    void clear();
    inline void setShaderType(Shader_e shaderNum){m_shaderNum = shaderNum;}
    inline const std::vector<float> &getVectVertex()const{return m_vertexBuffer;}
    inline const std::vector<uint32_t> &getVectIndices()const{return m_indices;}
    bool loadVertexColorComponent(const PositionVertexComponent *posComp,
                                  const ColorVertexComponent *colorComp);
    void loadVertexTextureComponent(const PositionVertexComponent &posComp,
                                    SpriteTextureComponent &spriteComp, bool drawByLine = false);
};
