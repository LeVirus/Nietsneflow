#ifndef VERTICESDATA_H
#define VERTICESDATA_H

#include <vector>
#include <stdint.h>

class VerticesData
{
private:
    std::vector<float> m_vertexBuffer;
    std::vector<uint32_t> m_indices;
public:
    VerticesData();
    void clear();
};

#endif // VERTICESDATA_H
