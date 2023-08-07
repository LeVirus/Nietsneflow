#include "ZoneLevelColl.hpp"
#include <cassert>

//===============================================================
ZoneLevelColl::ZoneLevelColl(const PairUI_t &size):m_size(size)
{
    m_zones.resize(m_size.first / m_zoneSize + 1);
    for(uint32_t i = 0; i < m_zones.size(); ++i)
    {
        m_zones[i].resize(m_size.second / m_zoneSize + 1);
    }
}

//===============================================================
void ZoneLevelColl::updateEntityToZones(uint32_t entityNum, const PairUI_t &coord)
{
    removeEntityToZones(entityNum);
    uint32_t x = coord.first / m_zoneSize,
            y = coord.second / m_zoneSize,
            modX = coord.first % m_zoneSize,
            modY = coord.second % m_zoneSize, xModif = x;
    bool limitX = false;
    addEntityToZone(entityNum, {x, y});
    if(modX == 0 && x != 0)
    {
        limitX = true;
        --xModif;
        addEntityToZone(entityNum, {x - 1, y});
    }
    else if(modX == 19 && x != m_size.first - 1)
    {
        limitX = true;
        addEntityToZone(entityNum, {x + 1, y});
        ++xModif;
    }
    if(modY == 0 && y != 0)
    {
        addEntityToZone(entityNum, {x, y - 1});
        if(limitX)
        {
            addEntityToZone(entityNum, {xModif, y - 1});
        }
    }
    else if(modY == 19 && y != m_size.second - 1)
    {
        addEntityToZone(entityNum, {x, y + 1});
        if(limitX)
        {
            addEntityToZone(entityNum, {xModif, y + 1});
        }
    }
}

//===============================================================
std::set<uint32_t> ZoneLevelColl::getEntitiesFromZones(uint32_t entityNum)const
{
    std::map<uint32_t, VectPairUi_t>::const_iterator it = m_cacheEntitiesZone.find(entityNum);
    if(it == m_cacheEntitiesZone.end())
    {
        return {};
    }
    std::set<uint32_t> set;
    uint32_t currentX, currentY;
    for(uint32_t i = 0; i < m_cacheEntitiesZone.at(entityNum).size(); ++i)
    {
        currentX = m_cacheEntitiesZone.at(entityNum)[i].first;
        currentY = m_cacheEntitiesZone.at(entityNum)[i].second;
        std::copy(m_zones[currentX][currentY].begin(),
                  m_zones[currentX][currentY].end(), std::inserter(set, set.end()));

    }
    return set;
}

//===============================================================
void ZoneLevelColl::addEntityToZone(uint32_t entityNum, const PairUI_t &zoneCoord)
{
    if(zoneCoord.first >= m_zones.size() || zoneCoord.second >= m_zones[zoneCoord.first].size())
    {
        return;
    }
    std::map<uint32_t, VectPairUi_t>::iterator it = m_cacheEntitiesZone.find(entityNum);
    if(it != m_cacheEntitiesZone.end())
    {
        m_cacheEntitiesZone.insert({entityNum, {}});
    }
    m_cacheEntitiesZone[entityNum].push_back(zoneCoord);
    m_zones[zoneCoord.first][zoneCoord.second].insert(entityNum);
}

//===============================================================
void ZoneLevelColl::removeEntityToZones(uint32_t entityNum, bool updateMode)
{
    std::map<uint32_t, VectPairUi_t>::iterator it = m_cacheEntitiesZone.find(entityNum);
    if(it == m_cacheEntitiesZone.end())
    {
        return;
    }
    uint32_t currentX, currentY;
    SetUi_t::iterator itt;
    for(uint32_t i = 0; i < it->second.size(); ++i)
    {
        currentX = m_cacheEntitiesZone[entityNum][i].first;
        currentY = m_cacheEntitiesZone[entityNum][i].second;
        itt = m_zones[currentX][currentY].find(entityNum);
        assert(itt != m_zones[currentX][currentY].end());
        m_zones[currentX][currentY].erase(itt);
    }
    if(!updateMode)
    {
        m_cacheEntitiesZone.erase(it);
    }
}
