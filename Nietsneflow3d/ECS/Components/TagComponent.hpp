#ifndef TAGCOMPONENT_HPP
#define TAGCOMPONENT_HPP

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct TagComponent : public ecs::Component
{
    TagComponent()
    {
        muiTypeComponent = Components_e::TAG_COMPONENT;
    }
    CollisionTag_e m_tag;
    virtual ~TagComponent() = default;
};

#endif // TAGCOMPONENT_HPP
