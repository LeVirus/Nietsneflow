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
    std::vector<uint32_t> m_indices;
    const Shader_e m_shaderNum;
    uint32_t m_cursor = 0;
    uint32_t m_ebo, m_vao, m_vbo;
private:
    void addIndices(BaseShapeType_e shapeType);
    void init();
    void genGLBuffers();
public:
    VerticesData(Shader_e shaderNum);
    void bindGLBuffers();
    bool bindGLVertexArray();
    void attribGLVertexPointer();
    void drawElement();
    void clear();
    void TESST(Shader &m_shader);
    inline const std::vector<float> &getVectVertex()const{return m_vertexBuffer;}
    inline const std::vector<uint32_t> &getVectIndices()const{return m_indices;}

    template<size_t SIZE>
    bool loadVertexComponent(const PositionVertexComponent<SIZE> *posComp,
                             const ColorVertexComponent<SIZE> *colorComp)
    {
        assert(posComp && "Position component is Null.");
        assert(colorComp && "Color component is Null.");
        if(m_shaderNum != Shader_e::CEILING_FLOOR)
        {
            return false;
        }
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
