#include <PictureData.hpp>
#include <limits>
#include <cassert>

#include <iostream>

//===================================================================
PictureData::PictureData()
{

}

//===================================================================
void PictureData::setTexturePath(const vectStr_t &vectTextures)
{
    m_vectTexturePath = vectTextures;
}

//===================================================================
void PictureData::setSpriteData(const SpriteData &spriteData, const std::string &identifier)
{
    m_mapIdentifier.insert({identifier, m_vectSpriteData.size()});
    m_vectSpriteData.emplace_back(spriteData);
}

//===================================================================
std::optional<uint8_t> PictureData::getIdentifier(const std::string &spriteName) const
{
    assert(m_upToDate && "PictureData isn't up to date.");
    std::map<std::string, uint8_t>::const_iterator it = m_mapIdentifier.begin();
    it = m_mapIdentifier.find(spriteName);
    if(it == m_mapIdentifier.end())
    {
        return std::nullopt;
    }
    return std::optional<uint8_t>{it->second};
}

//===================================================================
void PictureData::display()
{
    std::cout << "PictureData===================================" << std::endl;
    std::cout << "Texture path ::" << std::endl;
    for(uint32_t i = 0; i < m_vectTexturePath.size(); ++i)
    {
        std::cout << m_vectTexturePath[i] << "  " << std::endl;
    }

    std::cout << "GroundData ::" << std::endl;
//    std::cout << m_groundData.m_apparence << "\n" <<
//                 m_groundData.m_spriteNum << "\n" <<
//                 std::get<0>(m_groundData.m_tupleColor) << "  " <<
//                 std::get<1>(m_groundData.m_tupleColor) << "  " <<
//                 std::get<2>(m_groundData.m_tupleColor) << "  " <<
//                 std::endl;

//    std::cout << "CeilingData ::" << std::endl;
//    std::cout << m_ceilingData.m_apparence << "\n" <<
//                 m_ceilingData.m_spriteNum << "\n" <<
//                 std::get<0>(m_ceilingData.m_tupleColor) << "  " <<
//                 std::get<1>(m_ceilingData.m_tupleColor) << "  " <<
//                 std::get<2>(m_ceilingData.m_tupleColor) << "  " <<
//                 std::endl;

    std::cout << "Vect SpriteData ::" << std::endl;
    for(uint32_t i = 0; i < m_vectSpriteData.size(); ++i)
    {
        std::cout << m_vectSpriteData[i].m_textureNum << "\n";
        for(uint32_t j = 0; j <  m_vectSpriteData[i].m_texturePosVertex.size(); ++j)
        {
            std::cout << m_vectSpriteData[i].m_texturePosVertex[j].first << "  "
                      << m_vectSpriteData[i].m_texturePosVertex[j].second << "\n";
        }
    }
    std::cout << "END PictureData=============================" << std::endl;

}

//===================================================================
void PictureData::clear()
{
    m_vectTexturePath.clear();
    m_vectSpriteData.clear();
    m_mapIdentifier.clear();
    m_upToDate = false;
}

//===================================================================
void PictureData::setBackgroundData(const GroundCeilingData &ground, const GroundCeilingData &ceiling)
{
    m_groundData = ground;
    m_ceilingData = ceiling;
}

