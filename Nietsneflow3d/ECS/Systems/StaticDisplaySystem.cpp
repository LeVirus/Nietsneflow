#include "StaticDisplaySystem.hpp"
#include <ECS/Components/MemPositionsVertexComponents.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <cassert>

//===================================================================
StaticDisplaySystem::StaticDisplaySystem() : m_weaponVertice(Shader_e::TEXTURE_S)
{
    bAddComponentToSystem(Components_e::VISION_COMPONENT);
}

//===================================================================
void StaticDisplaySystem::execSystem()
{
    System::execSystem();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        writeVertexFromComponent(mVectNumEntity[i]);
        drawVertex();
    }
}

//===================================================================
void StaticDisplaySystem::writeVertexFromComponent(uint32_t numObserverEntity)
{
    PlayerConfComponent *playerComp = stairwayToComponentManager().
                searchComponentByType<PlayerConfComponent>(numObserverEntity,
                                                           Components_e::PLAYER_CONF_COMPONENT);
    assert(playerComp);
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(playerComp->m_weaponEntity,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(playerComp->m_weaponEntity,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    TimerComponent *timerComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(playerComp->m_weaponEntity,
                                                  Components_e::TIMER_COMPONENT);
    assert(timerComp);
    assert(posComp);
    assert(spriteComp);
    assert(!posComp->m_vertex.empty());
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
            timerComp->m_clock;
    //TMP
    if(m_currentWeaponSprite == WeaponsSpriteType_e::GUN_SHOOT &&
            elapsed_seconds.count() > 0.2)
    {
        setWeaponPlayer(playerComp->m_weaponEntity, WeaponsSpriteType_e::GUN_STATIC);
        playerComp->m_timerShootActive = false;
    }
    else if(playerComp->m_playerShoot)
    {
        timerComp->m_clock = std::chrono::system_clock::now();
        setWeaponPlayer(playerComp->m_weaponEntity, WeaponsSpriteType_e::GUN_SHOOT);
    }
    playerComp->m_playerShoot = false;
    m_weaponVertice.clear();
    m_weaponVertice.loadVertexStandartTextureComponent(*posComp, *spriteComp);
}

//===================================================================
void StaticDisplaySystem::drawVertex()
{
    m_shader->use();
    m_ptrVectTexture->operator[](static_cast<uint8_t>(m_numTextureWeapon)).bind();
    m_weaponVertice.confVertexBuffer();
    m_weaponVertice.drawElement();
}

//===================================================================
void StaticDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

//===================================================================
void StaticDisplaySystem::setWeaponPlayer(uint32_t weaponEntity, WeaponsSpriteType_e weaponSprite)
{
    m_currentWeaponSprite = weaponSprite;
    PositionVertexComponent *pos = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(weaponEntity,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    SpriteTextureComponent *spriteText = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(weaponEntity,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    MemSpriteDataComponent *memSprite = stairwayToComponentManager().
            searchComponentByType<MemSpriteDataComponent>(weaponEntity,
                                                          Components_e::MEM_SPRITE_DATA_COMPONENT);
    MemPositionsVertexComponents *memPosVertex = stairwayToComponentManager().
            searchComponentByType<MemPositionsVertexComponents>(weaponEntity,
                                                                Components_e::MEM_POSITIONS_VERTEX_COMPONENT);
    assert(pos);
    assert(spriteText);
    assert(memSprite);
    assert(memPosVertex);
    uint32_t index = static_cast<uint32_t>(weaponSprite);
    if(memPosVertex->m_vectSpriteData.empty())
    {
        return;
    }
    //set sprite
    spriteText->m_spriteData = memSprite->m_vectSpriteData[index];
    //set vertex pos
    pos->m_vertex.resize(memPosVertex->m_vectSpriteData[index].size());
    for(uint32_t i = 0; i < pos->m_vertex.size(); ++i)
    {
        pos->m_vertex[i] = memPosVertex->m_vectSpriteData[index][i];
    }

}
