#pragma once

#include "constants.hpp"
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
    void setShader(Shader &shader);
    void setWeaponSprite(uint32_t weaponEntity, WeaponsSpriteType_e weaponSprite);
    inline void setVectTextures(std::vector<Texture> &vectTexture)
    {
         m_ptrVectTexture = &vectTexture;
    }
    inline void setTextureWeaponNum(Texture_e numTexture)
    {
        m_numTextureWeapon = numTexture;
    }
    inline void setUnsetMenuActive(bool active)
    {
        m_menuActive = active;
    }
private:
    void fillWeaponMapEnum();
    void fillCursorMenuVertex(uint32_t playerEntity);
    void drawStandartSpriteVertex(SpriteTextureComponent *spriteComp, VertexID_e type);
    void confWriteVertex(WriteComponent *writeComp, PositionVertexComponent *posComp,
                         VertexID_e type);
    void drawWriteVertex(Texture_e numTexture, VertexID_e type);
    void treatWriteVertex(uint32_t numEntity, VertexID_e type);
    void drawLineWriteVertex(PositionVertexComponent *posComp, WriteComponent *writeComp, float fontSize);
    void confWeaponsVertexFromComponent(PlayerConfComponent *playerComp, SpriteTextureComponent *weaponSpriteComp);
    void setDisplayWeaponChange(PositionVertexComponent *posComp, PlayerConfComponent *playerComp,
                                MemPositionsVertexComponents *memPosComp);
    void setWeaponMovement(PlayerConfComponent *playerComp, PositionVertexComponent *posComp,
                           MemPositionsVertexComponents *memPosComp);
private:
    bool m_menuActive = false, m_cursorInit = false;
    Shader *m_shader;
    std::array<VerticesData, static_cast<uint32_t>(VertexID_e::TOTAL)> m_vertices;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    Texture_e m_numTextureWeapon;
    WeaponsSpriteType_e m_currentWeaponSprite;
    static std::map<WeaponsType_e, WeaponsSpriteType_e> m_weaponSpriteAssociated;
    float m_speedMoveWeaponChange = 0.05f, m_fontSizeStd = 0.06f, m_fontSizeMenu = 0.15f;
    pairFloat_t m_forkWeaponMovementX = {-0.4f, 0.1f}, m_forkWeaponMovementY = {-0.8f, -0.6f};
    float m_diffTotalDistanceMoveWeaponX = std::abs(m_forkWeaponMovementX.first -
                                                    m_forkWeaponMovementX.second);
    float m_middleWeaponMovementX = m_forkWeaponMovementX.first + (m_forkWeaponMovementX.second -
                                                                   m_forkWeaponMovementX.first) / 2.0f;
};

void modVertexPos(PositionVertexComponent *posComp, const pairFloat_t &mod);
