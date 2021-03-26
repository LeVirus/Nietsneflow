#pragma once

#include <OpenGLUtils/Shader.hpp>
#include <includesLib/BaseECS/system.hpp>
#include "constants.hpp"
#include <OpenGLUtils/Shader.hpp>
#include <OpenGLUtils/glheaders.hpp>
#include <OpenGLUtils/VerticesData.hpp>
#include <OpenGLUtils/Texture.hpp>

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
    void setWeaponPlayer(uint32_t weaponEntity, WeaponsSpriteType_e weaponSprite);
    inline void setVectTextures(std::vector<Texture> &vectTexture)
    {
         m_ptrVectTexture = &vectTexture;
    }
    inline void setTextureWeaponNum(Texture_e numTexture)
    {
        m_numTextureWeapon = numTexture;
    }
private:
    void drawVertex();
    void writeVertexFromComponent(uint32_t numObserverEntity);
private:
    Shader *m_shader;
    VerticesData m_weaponVertice;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    Texture_e m_numTextureWeapon;
};
