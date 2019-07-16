#ifndef POSITIONCOMP_HPP
#define POSITIONCOMP_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>
#include <functional>

struct PositionComponent : public ecs::Component
{
    PositionComponent()
    {
        muiTypeComponent = Components_e::POSITION_COMPONENT;
    }
    //Z axe isn't used in opengl context it will be always 0.0f
    //first represent x axe and second y axe
    std::pair<uint32_t, uint32_t> m_pairPosition;
    virtual ~PositionComponent() = default;
};

#endif // POSITIONCOMP_HPP

