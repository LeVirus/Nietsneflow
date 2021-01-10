#include "Level.hpp"
#include "constants.hpp"
#include <ECS/Components/MapCoordComponent.hpp>
#include <ECS/Components/MoveableComponent.hpp>
#include <iostream>

float Level::m_rangeViewPX;
pairUI_t Level::m_size;

//===================================================================
Level::Level()
{
    m_rangeViewPX = 100.0f;
}

//===================================================================
void Level::setPlayerInitData(const pairFloat_t &pairInitPlayerPos,
                              Direction_e playerDir)
{
    m_playerDeparture = pairInitPlayerPos;
    m_playerDepartureDirection = playerDir;
}

//===================================================================
void Level::display()
{
    std::cout << "LEVEL===================" << std::endl;
    std::cout << "Size ::" << std::endl;
    std::cout << "X :: " << m_size.first << "Y :: "
              << m_size.second <<  std::endl;
    std::cout << "Player departure ::" << std::endl;
//    std::cout << "X :: " << m_playerDeparture.first <<
//                 "  Y :: " << m_playerDeparture.second <<
//              "  Player Direction " << m_playerDepartureDirection << std::endl;
    for(uint32_t i = 0; i < m_groundElement.size(); ++i)
    {
        std::cout << "Vect Ground Element   \n"
                  << "Sprite Num :: " << m_groundElement.back().m_numSprite
                  << "  Sprite Size:: "
                  << "X :: "  << m_groundElement.back().m_inGameSpriteSize.first
                  << "  Y :: "  << m_groundElement.back().m_inGameSpriteSize.second
                  << "  traversable :: " << m_groundElement.back().m_traversable<< std::endl;
        for(uint32_t j = 0; j < m_groundElement.back().m_TileGamePosition.size(); ++j)
        {
            std::cout << "tile positions :: "
                    << "X :: "  << m_groundElement.back().m_TileGamePosition[j].first
                    << "  Y :: "  << m_groundElement.back().m_TileGamePosition[j].second
                    << std::endl;
        }
    }
    for(uint32_t i = 0; i < m_ceilingElement.size(); ++i)
    {
        std::cout << "Vect Ceiling Element   \n"
                  << "Sprite Num :: " << m_ceilingElement.back().m_numSprite
                  << "  Sprite Size:: "
                  << "X :: "  << m_ceilingElement.back().m_inGameSpriteSize.first
                  << "  Y :: "  << m_ceilingElement.back().m_inGameSpriteSize.second
                  << std::endl;
        for(uint32_t j = 0; j < m_ceilingElement.back().m_TileGamePosition.size(); ++j)
        {
            std::cout << "tile positions :: "
                    << "X :: "  << m_ceilingElement.back().m_TileGamePosition[j].first
                    << "  Y :: "  << m_ceilingElement.back().m_TileGamePosition[j].second
                    << std::endl;
        }
    }
    for(uint32_t i = 0; i < m_objectElement.size(); ++i)
    {
        std::cout << "Vect Object Element  \n"
                  << "Sprite Num :: " << m_objectElement.back().m_numSprite
                  << "  Sprite Size:: "
                  << "X :: "  << m_objectElement.back().m_inGameSpriteSize.first
                  << "  Y :: "  << m_objectElement.back().m_inGameSpriteSize.second
                  << std::endl;
        for(uint32_t j = 0; j < m_objectElement.back().m_TileGamePosition.size(); ++j)
        {
            std::cout << "tile positions :: "
                    << "X :: "  << m_objectElement.back().m_TileGamePosition[j].first
                    << "  Y :: "  << m_objectElement.back().m_TileGamePosition[j].second
                    << std::endl;
        }
    }

    for(uint32_t i = 0; i < m_wallData.size(); ++i)
    {
        std::cout << "Vect Wall Element  \n"
                  << "Sprite Num :: " << m_wallData.back().m_sprites[0]
                  << std::endl;
        for(uint32_t j = 0; j < m_wallData.back().m_TileGamePosition.size(); ++j)
        {
            std::cout << "tile positions :: "
                    << "X :: "  << m_wallData.back().m_TileGamePosition[j].first
                    << "  Y :: "  << m_wallData.back().m_TileGamePosition[j].second
                    << std::endl;
        }
    }

    std::cout << "Vect Enemy Element  \n";
    for(uint32_t i = 0; i < m_enemyData.size(); ++i)
    {
        std::cout << "  Attack Sprite:: ";
        for(uint32_t j = 0; j < m_enemyData.back().m_attackSprites.size(); ++j)
        {
            std::cout << m_enemyData.back().m_attackSprites[j] << ", ";
        }
        std::cout << "  \nDying Sprite:: ";
        for(uint32_t j = 0; j < m_enemyData.back().m_dyingSprites.size(); ++j)
        {
            std::cout << m_enemyData.back().m_dyingSprites[j] << ", ";
        }
        std::cout << "  \nMove Sprite:: ";
        for(uint32_t j = 0; j < m_enemyData.back().m_moveSprites.size(); ++j)
        {
            std::cout << m_enemyData.back().m_moveSprites[j] << ", ";
        }
        std::cout << "  \nStatic Sprite:: ";
        for(uint32_t j = 0; j < m_enemyData.back().m_staticSprites.size(); ++j)
        {
            std::cout << m_enemyData.back().m_staticSprites[j] << ", ";
        }
        std::cout << std::endl;

                 std::cout << "  Sprite Size:: "
                  << "X :: "  << m_enemyData.back().m_inGameSpriteSize.first
                  << "  Y :: "  << m_enemyData.back().m_inGameSpriteSize.second
        << "  traversable :: " << m_groundElement.back().m_traversable<< std::endl;

        for(uint32_t j = 0; j < m_enemyData.back().m_TileGamePosition.size(); ++j)
        {
            std::cout << "tile positions :: "
                    << "X :: "  << m_enemyData.back().m_TileGamePosition[j].first
                    << "  Y :: "  << m_enemyData.back().m_TileGamePosition[j].second
                    << std::endl;
        }
    }
    std::cout << "END LEVEL===================" << std::endl;
}

//===================================================================
pairFloat_t getAbsolutePosition(const pairUI_t &coord)
{
    return {static_cast<float>(coord.first) * LEVEL_TILE_SIZE_PX,
                static_cast<float>(coord.second) * LEVEL_TILE_SIZE_PX};
}

//===================================================================
pairUI_t getLevelCoord(pairFloat_t &position)
{
    if(position.first < 0.0f)
    {
        position.first = 0.0f;
    }
    if(position.second < 0.0f)
    {
        position.second = 0.0f;
    }
    return {static_cast<uint32_t>(position.first / LEVEL_TILE_SIZE_PX),
                static_cast<uint32_t>(position.second / LEVEL_TILE_SIZE_PX)};
}
