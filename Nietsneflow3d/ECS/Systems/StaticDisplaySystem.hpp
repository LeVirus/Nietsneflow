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
    void drawVertex();
    void writeWeaponsVertexFromComponent(uint32_t numObserverEntity);
    void setDisplayWeaponChange(PositionVertexComponent *posComp, PlayerConfComponent *playerComp,
                                MemPositionsVertexComponents *memPosComp);
    void setWeaponMovement(PlayerConfComponent *playerComp, PositionVertexComponent *posComp,
                           MemPositionsVertexComponents *memPosComp);
private:
    Shader *m_shader;
    VerticesData m_weaponVertice;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    Texture_e m_numTextureWeapon;
    WeaponsSpriteType_e m_currentWeaponSprite;
    static std::map<WeaponsType_e, WeaponsSpriteType_e> m_weaponSpriteAssociated;
    float m_speedMoveWeaponChange = 0.05f;
    pairFloat_t m_forkWeaponMovementX = {-0.4f, 0.1f}, m_forkWeaponMovementY = {-0.8f, -0.6f};
    float m_middleWeaponMovementX = m_forkWeaponMovementX.first + (m_forkWeaponMovementX.second -
                                                                   m_forkWeaponMovementX.first) / 2.0f;
};

void modVertexPos(PositionVertexComponent *posComp, const pairFloat_t &mod);