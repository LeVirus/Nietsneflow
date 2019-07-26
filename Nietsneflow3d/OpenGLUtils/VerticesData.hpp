#ifndef VERTICESDATA_H
#define VERTICESDATA_H

#include <vector>
#include <stdint.h>
#include <constants.hpp>
#include <cassert>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/ColorVertexComponent.hpp>

class Shader;

class VerticesData
{
private:
    std::vector<float> m_vertexBuffer;
    std::vector<uint32_t> m_indices, m_shaderInterpretData;
    const Shader_e m_shaderNum;
    uint32_t m_cursor = 0, m_sizeOfVertex;
    uint32_t m_ebo, m_vao, m_vbo;
private:
    void addIndices(BaseShapeType_e shapeType);
    void init();
    void genGLBuffers();
    void setVectGLPointer();
    void bindGLBuffers();
    void attribGLVertexPointer();
public:
    VerticesData(Shader_e shaderNum);
    void confVertexBuffer();
    void drawElement();
    void clear();
    inline const std::vector<float> &getVectVertex()const{return m_vertexBuffer;}
    inline const std::vector<uint32_t> &getVectIndices()const{return m_indices;}

    template<size_t SIZE>
    bool loadVertexComponent(const PositionVertexComponent<SIZE> *posComp,
                             const ColorVertexComponent<SIZE> *colorComp)
    {
        if(m_shaderNum != Shader_e::COLOR_S)
        {
            return false;
        }
        assert(posComp && "Position component is Null.");
        assert(colorComp && "Color component is Null.");
        for(uint32_t j = 0; j < SIZE; ++j)
        {
            m_vertexBuffer.emplace_back(posComp->m_vertex[j].first);
            m_vertexBuffer.emplace_back(posComp->m_vertex[j].second);
            m_vertexBuffer.emplace_back(std::get<0>(colorComp->m_vertex[j]));
            m_vertexBuffer.emplace_back(std::get<1>(colorComp->m_vertex[j]));
            m_vertexBuffer.emplace_back(std::get<2>(colorComp->m_vertex[j]));
        }
        BaseShapeType_e shapeType = (SIZE == 3 ? BaseShapeType_e::TRIANGLE :
                                                 BaseShapeType_e::RECTANGLE);
        addIndices(shapeType);
        return true;
    }
};

#endif // VERTICESDATA_H
