#ifndef MOVEABLECOMPONENT_HPP
#define MOVEABLECOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <functional>

using pairFloat_t = std::pair<float, float>;

struct MoveableComponent : public ecs::Component
{
    MoveableComponent()
    {
        muiTypeComponent = Components_e::MOVEABLE_COMPONENT;
    }
    int32_t m_degreeOrientation;
    float m_velocity = 3.0f;
    virtual ~MoveableComponent() = default;
};

#endif // MOVEABLECOMPONENT_HPP
