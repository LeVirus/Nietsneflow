#include "FontData.hpp"
#include <iostream>
#include <cassert>

//===================================================================
FontData::FontData()
{

}

//===================================================================
void FontData::addCharSpriteData(const SpriteData &spriteData, const std::string &identifier)
{
    m_mapFontData.insert({identifier[0], spriteData});
}

//===================================================================
void FontData::clear()
{
    m_mapFontData.clear();
}

//===================================================================
VectSpriteDataRef_t FontData::getWriteData(const std::string &str, uint32_t &numTexture)const
{
    if(str.empty())
    {
        return {};
    }
    bool textureNumDefined = false;
    VectSpriteDataRef_t vect;
    vect.reserve(str.size());
    std::map<char, SpriteData>::const_iterator it;
    for(uint32_t i = 0; i < str.size(); ++i)
    {
        it = m_mapFontData.find(str[i]);
        if(it != m_mapFontData.end())
        {
            vect.emplace_back(const_cast<SpriteData&>(it->second));
            if(!textureNumDefined)
            {
                numTexture = it->second.m_textureNum;
                textureNumDefined = true;
            }
        }
    }
    return vect;
}
