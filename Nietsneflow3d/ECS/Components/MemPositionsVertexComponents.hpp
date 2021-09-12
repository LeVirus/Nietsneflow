#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <PictureData.hpp>
#include <vector>

/**
 * @brief The MemSpriteDataComponent struct stores all used sprite
 * as pointers in the case of animated entities.
 */
struct MemPositionsVertexComponents : public ecs::Component
{
    MemPositionsVertexComponents()
    {
        muiTypeComponent = Components_e::MEM_POSITIONS_VERTEX_COMPONENT;
    }
    std::vector<std::array<PairFloat_t, 4>> m_vectSpriteData;
    virtual ~MemPositionsVertexComponents() = default;
};
