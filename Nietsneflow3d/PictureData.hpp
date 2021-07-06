#pragma once

#include <vector>
#include <map>
#include <array>
#include <string>
#include <tuple>
#include <constants.hpp>
#include <stdint.h>

using tupleFloat_t = std::tuple<float, float, float>;
using tupleDouble_t = std::tuple<double, double, double>;
using vectStr_t = std::vector<std::string>;

/**
 * @brief The DecorData struct Struct representing data for ground or ceiling for
 * FPS rendering.
 */
struct GroundCeilingData
{
    DisplayType_e m_apparence /*= E_DISPLAY_TYPE::COLOR*/;
    uint8_t m_spriteNum;
    tupleFloat_t m_tupleColor;
};

/**
 * @brief The SpriteData struct store all needed data to display a static sprite.
 */
struct SpriteData
{
    uint32_t m_textureNum;
    /*
     * 0 top left
     * 1 top right
     * 2 bottom right
     * 3 bottom left
    */
    std::array<pairFloat_t, 4> m_texturePosVertex;
    SpriteData& operator=(const SpriteData& other)
    {
        m_textureNum = other.m_textureNum;
        m_texturePosVertex = other.m_texturePosVertex;
        return *this;
    }
};


class PictureData
{
public:
    PictureData();
    void setTexturePath(const vectStr_t &vectTextures);
    void setGroundAndCeilingData(const std::array<GroundCeilingData, 2> &arrayGAndCData);
    void setSpriteData(const SpriteData &spriteData, const std::string &identifier);
    std::optional<uint8_t> getIdentifier(const std::string &spriteName)const;
    void display();
    void clear();
    inline void setUpToDate(){m_upToDate = true;}
    inline const vectStr_t &getTexturePath()const {return m_vectTexturePath;}
    inline const std::vector<SpriteData> &getSpriteData()const {return m_vectSpriteData;}
    inline const GroundCeilingData &getCeilingData()const {return m_ceilingData;}
    inline const GroundCeilingData &getGroundData()const {return m_groundData;}
private:
    vectStr_t m_vectTexturePath;
    GroundCeilingData m_groundData, m_ceilingData;
    std::vector<SpriteData> m_vectSpriteData;
    std::map<std::string, uint8_t> m_mapIdentifier;
    bool m_upToDate = false;
};
