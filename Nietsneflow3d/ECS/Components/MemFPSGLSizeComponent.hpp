#pragma once

#include <BaseECS/component.hpp>
#include <constants.hpp>

struct MemFPSGLSizeComponent : public ecs::Component
{
    MemFPSGLSizeComponent()
    {
        muiTypeComponent = Components_e::MEM_FPS_GLSIZE_COMPONENT;
    }
    std::vector<PairFloat_t> m_memGLSizeData;
    virtual ~MemFPSGLSizeComponent() = default;
};
