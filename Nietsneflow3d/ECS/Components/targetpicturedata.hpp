#ifndef TARGETPICTUREDATA_HPP
#define TARGETPICTUREDATA_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <array>
#include <functional>

using pairFloat_t = std::pair<float, float>;
using vertexArray_t = std::array<pairFloat_t, 4>;

/**
 * @brief The TargetPictureDataComp struct
 * Coordinated data for OpenGL displaying.
 */
struct TargetPictureDataComp : public ecs::Component
{
    TargetPictureDataComp()
    {
        muiTypeComponent = Components_e::TARGET_PICTURE_DATA_COMPONENT;
    }

    vertexArray_t m_vertex;
    virtual ~TargetPictureDataComp() = default;
};

#endif // TARGETPICTUREDATA_HPP

