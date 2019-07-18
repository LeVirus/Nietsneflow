#ifndef TARGETPICTUREDATA_HPP
#define TARGETPICTUREDATA_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <functional>

using pairFloat_t = std::pair<float, float>;
using vertexArray_t = std::array<pairFloat_t, 4>;

/**
 * @brief The VertexComponent struct
 * Coordinated data for OpenGL displaying.
 */
struct PositionVertexComponent : public ecs::Component
{
    PositionVertexComponent()
    {
        muiTypeComponent = Components_e::POSITION_VERTEX_COMPONENT;
    }

    vertexArray_t m_vertex;
    virtual ~PositionVertexComponent() = default;
};

#endif // TARGETPICTUREDATA_HPP

