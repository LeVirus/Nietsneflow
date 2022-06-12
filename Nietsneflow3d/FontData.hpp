#pragma once

#include <PictureData.hpp>
#include <map>
#include <functional>

using VectSpriteDataRef_t = std::vector<std::reference_wrapper<SpriteData>>;
using VectVectSpriteDataRef_t = std::vector<VectSpriteDataRef_t>;
class FontData
{
public:
    FontData();
    void addCharSpriteData(const SpriteData &spriteData, const std::string &identifier);
    void clear();
    VectSpriteDataRef_t getWriteData(const std::string &str, uint32_t &numTexture)const;
private:
    std::map<char, SpriteData> m_mapFontData;
};
