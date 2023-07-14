#pragma once

#include <vector>
#include <set>
#include <map>
#include <cstdint>
#include <constants.hpp>
#include <iostream>

using SetUi_t = std::set<uint32_t>;
using VectSetUi_t = std::vector<SetUi_t>;
using VectPairUi_t = std::vector<PairUI_t>;

class ZoneLevelColl
{
public:
    ZoneLevelColl(const PairUI_t &size);
    void updateEntityToZones(uint32_t entityNum, const PairUI_t &coord);
    void removeEntityToZones(uint32_t entityNum, bool updateMode = false);
    std::set<uint32_t> getEntitiesFromZones(uint32_t entityNum) const;
private:
    void addEntityToZone(uint32_t entityNum, const PairUI_t &zoneCoord);
private:
    PairUI_t m_size;
    uint32_t m_zoneSize = 20;
    std::vector<VectSetUi_t> m_zones;
    //first lateral, second vertical
    std::map<uint32_t, VectPairUi_t> m_cacheEntitiesZone;
};
