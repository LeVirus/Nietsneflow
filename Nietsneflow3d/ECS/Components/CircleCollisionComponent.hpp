#ifndef CIRCLECOLLISIONCOMPONENT_HPP
#define CIRCLECOLLISIONCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct CircleCollisionComponent : public ecs::Component
{
    CircleCollisionComponent()
    {
        muiTypeComponent = Components_e::CIRCLE_COLLISION_COMPONENT;
    }
    std::pair<float, float> m_center;
    float m_ray;
    virtual ~CircleCollisionComponent() = default;
};

#endif // CIRCLECOLLISIONCOMPONENT_HPP

