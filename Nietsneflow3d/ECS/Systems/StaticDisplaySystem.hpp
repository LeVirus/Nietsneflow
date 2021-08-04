#pragma once

#include "constants.hpp"
#include "FontData.hpp"
#include <OpenGLUtils/Shader.hpp>
#include <includesLib/BaseECS/system.hpp>
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>
#include <map>


struct TimerComponent;
struct PlayerConfComponent;
struct MemPositionsVertexComponents;
struct WriteComponent;

enum class VertexID_e
{
    WEAPON,
    LIFE_WRITE,
    AMMO_WRITE,
    MENU_WRITE,
    MENU_CURSOR,
    TOTAL
};

/**
 * @brief The StaticDisplaySystem class displays the menu
 * and the weapon in use.
 */
class StaticDisplaySystem : public ecs::System
{
public:
    StaticDisplaySystem();
    void execSystem()override;
    void displayMenu();
    void setShader(Shader &shader);
    void setWeaponSprite(uint32_t weaponEntity, WeaponsSpriteType_e weaponSprite);
    inline void setVectTextures(std::vector<Texture> &vectTexture)
    {
        m_ptrVectTexture = &vectTexture;
    }
    inline void memFontDataPtr(FontData const *fontData)
    {
        m_fontDataPtr = fontData;
    }
    inline const std::map<WeaponsType_e, WeaponsSpriteType_e> &getWeaponsSpriteAssociated()const
    {
        return m_weaponSpriteAssociated;
    }
private:
    void fillCursorMenuVertex(PlayerConfComponent *playerComp);
    void updateMenuCursorPosition(PlayerConfComponent *playerComp);
    void confWriteVertex(WriteComponent *writeComp, PositionVertexComponent *posComp,
                         VertexID_e type);
    void drawVertex(uint32_t numTexture, VertexID_e type);
    void treatWriteVertex(uint32_t numEntity, VertexID_e type, const std::string &value = "");
    void drawLineWriteVertex(PositionVertexComponent *posComp, WriteComponent *writeComp);
    void confWeaponsVertexFromComponent(PlayerConfComponent *playerComp, SpriteTextureComponent *weaponSpriteComp);
    void treatWeaponShootAnimation(float elapsedSeconds, PlayerConfComponent *playerComp,
                                    TimerComponent *timerComp, WeaponsType_e weapon);
    void setDisplayWeaponChange(PositionVertexComponent *posComp, PlayerConfComponent *playerComp,
                                MemPositionsVertexComponents *memPosComp);
    void setWeaponMovement(PlayerConfComponent *playerComp, PositionVertexComponent *posComp,
                           MemPositionsVertexComponents *memPosComp);
private:
    FontData const *m_fontDataPtr;
    bool m_cursorInit = false;
    Shader *m_shader;
    std::array<VerticesData, static_cast<uint32_t>(VertexID_e::TOTAL)> m_vertices;
    std::array<double, static_cast<uint32_t>(WeaponsType_e::TOTAL)> m_weaponsLatences =
    {0.08, 0.12, 0.15, 0.1};
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    WeaponsSpriteType_e m_currentWeaponSprite;
    std::map<WeaponsType_e, WeaponsSpriteType_e> m_weaponSpriteAssociated;
    float m_speedMoveWeaponChange = 0.05f;
    pairFloat_t m_forkWeaponMovementX = {-0.4f, 0.1f}, m_forkWeaponMovementY = {-0.8f, -0.6f};
    float m_diffTotalDistanceMoveWeaponX = std::abs(m_forkWeaponMovementX.first -
                                                    m_forkWeaponMovementX.second);
    float m_middleWeaponMovementX = m_forkWeaponMovementX.first + (m_forkWeaponMovementX.second -
                                                                   m_forkWeaponMovementX.first) / 2.0f;
    //FORCE UPDATE AT LAUNCH
    CurrentMenuCursorPos_e m_currentCursorPos = CurrentMenuCursorPos_e::TOTAL;
};

void modVertexPos(PositionVertexComponent *posComp, const pairFloat_t &mod);
