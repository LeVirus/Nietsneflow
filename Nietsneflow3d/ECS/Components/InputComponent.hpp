#ifndef INPUTCOMPONENT_HPP
#define INPUTCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct InputComponent : public ecs::Component
{
    InputComponent()
    {
        muiTypeComponent = Components_e::INPUT_COMPONENT;
    }
    uint32_t m_controlMode = 0;
    virtual ~InputComponent() = default;
};

#endif // INPUTCOMPONENT_HPP

