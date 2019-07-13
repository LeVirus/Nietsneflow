#ifndef PICTUREDATA_H
#define PICTUREDATA_H

#include <vector>
#include <map>
#include <array>
#include <string>
#include <tuple>
#include <constants.hpp>
#include <stdint.h>

using tupleFloat_t = std::tuple<float, float, float>;
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
    uint8_t m_textureNum;
    std::pair<float, float> m_pairTexturePosition;
    std::pair<float, float> m_pairTextureSize;
};


class PictureData
{
private:
    vectStr_t m_vectTexturePath;
    GroundCeilingData m_groundData, m_ceilingData;
    std::vector<SpriteData> m_vectSpriteData;
    std::map<std::string, uint8_t> m_mapIdentifier;
    bool m_upToDate = false;
public:
    PictureData();
    void setTexturePath(const vectStr_t &vectTextures);
    void setGroundAndCeilingData(const std::array<GroundCeilingData, 2> &arrayGAndCData);
    void setSpriteData(const SpriteData &spriteData, const std::string &identifier);
    uint8_t getIdentifier(const std::string &spriteName)const;
    void display();
    void clear();
    inline void setUpToDate(){m_upToDate = true;}
};



#endif // PICTUREDATA_H
