#ifndef TARGETPICTUREDATA_HPP
#define TARGETPICTUREDATA_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <functional>

using pairFloat_t = std::pair<float, float>;

/**
 * @brief The VertexComponent struct
 * Coordinated data for OpenGL displaying.
 */
template<size_t SIZE>
struct PositionVertexComponent : public ecs::Component
{
    PositionVertexComponent()
    {
        muiTypeComponent = Components_e::POSITION_VERTEX_COMPONENT;
    }

    std::array<pairFloat_t, SIZE> m_vertex;
    virtual ~PositionVertexComponent() = default;
};

#endif // TARGETPICTUREDATA_HPP

