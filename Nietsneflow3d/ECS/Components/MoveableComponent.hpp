#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <functional>
#include <optional>

using PairFloat_t = std::pair<float, float>;

struct MoveableComponent : public ecs::Component
{
    MoveableComponent()
    {
        muiTypeComponent = Components_e::MOVEABLE_COMPONENT;
    }
    float m_degreeOrientation;
    //used for movement (forward backward strafe...)
    float m_currentDegreeMoveDirection;
    float m_velocity = 3.0f;
    float m_rotationAngle = 3.000f;
    //first eject velocity, SECOND Time
    std::optional<std::pair<float, double>> m_ejectData = std::nullopt;
    //first direction, second entity, third vertical == true or lateral == false
    virtual ~MoveableComponent() = default;
};
