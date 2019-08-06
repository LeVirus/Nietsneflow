#ifndef MAPDISPLAYSYSTEM_H
#define MAPDISPLAYSYSTEM_H

#include <BaseECS/system.hpp>

class MapDisplaySystem : public ecs::System
{
private:
    void setUsedComponents();
public:
    MapDisplaySystem();
};

#endif // MAPDISPLAYSYSTEM_H
