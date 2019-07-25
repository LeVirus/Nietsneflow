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
void VerticesData::init()
{
    genGLBuffers();
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
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_indices.size(),
                 &m_indices[0], GL_STATIC_DRAW);
}

//===================================================================
void VerticesData::attribGLVertexPointer()
{
    std::vector<uint32_t> vectDataVertex;
    switch (m_shaderNum)
    {
    case Shader_e::CEILING_FLOOR:
        vectDataVertex = {2,3};
        break;
    case Shader_e::WALL:
//        vectDataVertex = {2,3};
        break;
    case Shader_e::STATIC_ELEMENT:
//        vectDataVertex = {2,3};
        break;
    case Shader_e::TOTAL_SHADER:
        assert("Incoherant shader enum.");
    }
    uint32_t totalSize = std::accumulate(vectDataVertex.begin(),
                                         vectDataVertex.end(), 0);
    uint32_t offset = 0;
    for(uint32_t i = 0; i < vectDataVertex.size(); ++i)
    {
        glVertexAttribPointer(i, vectDataVertex[i], GL_FLOAT, GL_FALSE,
                              totalSize * sizeof(float), (void*)offset);
        offset = vectDataVertex[i] * sizeof(float);
        glEnableVertexAttribArray(i);
    }
}

//===================================================================
bool VerticesData::bindGLVertexArray()
{
    if(m_vertexBuffer.empty() || m_indices.empty())
    {
        return false;
    }
    glBindVertexArray(m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertexBuffer.size(),
                 &m_vertexBuffer[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_indices.size(),
                 &m_indices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    //    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);
}

//===================================================================
void VerticesData::drawElement()
{
//    m_shader->use();
    glBindVertexArray(m_vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
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

//void VerticesData::TESST(Shader &m_shader)
//{
////        uint32_t EBO;
////        //1 number of buffer to generate
////        glGenBuffers(1, &EBO);

////        uint32_t VBO, VAO;
////        glGenVertexArrays(1, &VAO);
////        glGenBuffers(1, &VBO);
//        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
////        glBindVertexArray(m_vao);


////        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
////        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertexBuffer.size(),
////                     &m_vertexBuffer[0], GL_STATIC_DRAW);
////    //    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

////        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
////    //    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
////        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_indices.size(),
////                     &m_indices[0], GL_STATIC_DRAW);

////        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
////        glEnableVertexAttribArray(0);

////        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
////        glEnableVertexAttribArray(1);
//        // note that this is allowed, the call to glVertexAttribPointer registered m_vbo as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
////        glBindBuffer(GL_ARRAY_BUFFER, 0);

//        // You can unbind the VAO afterwards so other m_vao calls won't accidentally modify this m_vao, but this rarely happens. Modifying other
//        // m_vaos requires a call to glBindVertexArray anyways so we generally don't unbind m_vaos (nor m_vbos) when it's not directly necessary.
////        glBindVertexArray(0);
//        m_shader.use();

//        // draw our first triangle
//        glBindVertexArray(m_vao); // seeing as we only have a single m_vao there's no need to bind it every time, but we'll do so to keep things a bit more organized
//        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
//    //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//    //    glDrawArrays(GL_TRIANGLES, 0, 4);
//    //    glDrawArrays(GL_TRIANGLES, 4, 4);

//        glBindVertexArray(0); // no need to unbind it every time
//}

