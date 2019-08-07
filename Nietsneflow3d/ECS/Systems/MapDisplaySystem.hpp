#ifndef MAPDISPLAYSYSTEM_H
#define MAPDISPLAYSYSTEM_H

#include <BaseECS/system.hpp>
#include <functional>

class MapDisplaySystem : public ecs::System
{
private:
    void setUsedComponents();
public:
    MapDisplaySystem();
    std::pair<uint32_t, uint32_t> m_coordLevel;
};

#endif // MAPDISPLAYSYSTEM_H
