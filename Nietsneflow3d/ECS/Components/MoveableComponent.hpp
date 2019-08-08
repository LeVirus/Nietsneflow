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
    pairFloat_t m_absoluteMapPosition;
    float m_degreeOrientation;
    virtual ~MoveableComponent() = default;
};

#endif // MOVEABLECOMPONENT_HPP

