#pragma once

#include <PictureData.hpp>
#include <map>
#include <functional>

using VectSpriteDataRef_t = std::vector<std::reference_wrapper<SpriteData>>;
using VectVectSpriteDataRef_t = std::vector<VectSpriteDataRef_t>;
using MapSpriteData_t = std::map<char, SpriteData>;

struct WriteComponent;

enum class Font_e
{
    STANDARD,
    SELECTED,
    BASE,
    TOTAL
};

class FontData
{
public:
    FontData();
    void addCharSpriteData(const SpriteData &spriteData, const std::string &identifier, Font_e type);
    void clear();
    VectSpriteDataRef_t getWriteData(const std::string &str, WriteComponent *writeComp, Font_e type)const;
private:
    std::array<MapSpriteData_t, static_cast<uint32_t>(Font_e::TOTAL)> m_mapFontData;
};
