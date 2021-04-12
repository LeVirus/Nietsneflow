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
private:
    void fillWeaponMapEnum();
    void drawVertex(uint32_t observerEntity);
    void drawWriteVertex(uint32_t observerEntity);
    void drawLineWriteVertex(PositionVertexComponent *posComp, WriteComponent *writeComp);
    void writeWeaponsVertexFromComponent(uint32_t numObserverEntity);
    void setDisplayWeaponChange(PositionVertexComponent *posComp, PlayerConfComponent *playerComp,
                                MemPositionsVertexComponents *memPosComp);
    void setWeaponMovement(PlayerConfComponent *playerComp, PositionVertexComponent *posComp,
                           MemPositionsVertexComponents *memPosComp);
private:
    Shader *m_shader;
    std::array<VerticesData, static_cast<uint32_t>(VertexID_e::TOTAL)> m_vertices;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    Texture_e m_numTextureWeapon;
    WeaponsSpriteType_e m_currentWeaponSprite;
    static std::map<WeaponsType_e, WeaponsSpriteType_e> m_weaponSpriteAssociated;
    float m_speedMoveWeaponChange = 0.05f, m_fontSize = 0.2f;
    pairFloat_t m_forkWeaponMovementX = {-0.4f, 0.1f}, m_forkWeaponMovementY = {-0.8f, -0.6f};
    float m_diffTotalDistanceMoveWeaponX = std::abs(m_forkWeaponMovementX.first -
                                                    m_forkWeaponMovementX.second);
    float m_middleWeaponMovementX = m_forkWeaponMovementX.first + (m_forkWeaponMovementX.second -
                                                                   m_forkWeaponMovementX.first) / 2.0f;
};

void modVertexPos(PositionVertexComponent *posComp, const pairFloat_t &mod);
