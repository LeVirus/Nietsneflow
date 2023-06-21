#include "FontData.hpp"
#include <iostream>
#include <cassert>
#include <ECS/Components/WriteComponent.hpp>

//===================================================================
FontData::FontData()
{

}

//===================================================================
void FontData::addCharSpriteData(const SpriteData &spriteData, const std::string &identifier, Font_e type)
{
    uint32_t i = static_cast<uint32_t>(type);
    m_mapFontData[i].insert({identifier[0], spriteData});
}

//===================================================================
void FontData::clear()
{
    for(uint32_t i = 0; i < m_mapFontData.size(); ++i)
    {
        m_mapFontData[i].clear();
    }
}

//===================================================================
VectSpriteDataRef_t FontData::getWriteData(const std::string &str, WriteComponent &writeComp, Font_e type)const
{
    if(str.empty())
    {
        return {};
    }
    uint32_t type_i = static_cast<uint32_t>(type);
    VectSpriteDataRef_t vect;
    vect.reserve(str.size());
    std::map<char, SpriteData>::const_iterator it;
    writeComp.m_fontType = type;
    for(uint32_t i = 0; i < str.size(); ++i)
    {
        it = m_mapFontData[type_i].find(str[i]);
        if(str[i] != ' ' && str[i] != '\\' && it == m_mapFontData[type_i].end())
        {
            it = m_mapFontData[type_i].find('?');
        }
        if(it != m_mapFontData[type_i].end())
        {
            vect.emplace_back(const_cast<SpriteData&>(it->second));
            if(i == 0)
            {
                writeComp.m_numTexture = it->second.m_textureNum;
            }
        }
    }
    return vect;
}
