#include "StaticDisplaySystem.hpp"
#include <ECS/Components/MemPositionsVertexComponents.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <cassert>

std::map<WeaponsType_e, WeaponsSpriteType_e> StaticDisplaySystem::m_weaponSpriteAssociated;

//===================================================================
StaticDisplaySystem::StaticDisplaySystem() : m_weaponVertice(Shader_e::TEXTURE_S)
{
    fillWeaponMapEnum();
    bAddComponentToSystem(Components_e::VISION_COMPONENT);
}

//===================================================================
void StaticDisplaySystem::fillWeaponMapEnum()
{
    m_weaponSpriteAssociated.insert({WeaponsType_e::GUN, WeaponsSpriteType_e::GUN_STATIC});
    m_weaponSpriteAssociated.insert({WeaponsType_e::SHOTGUN, WeaponsSpriteType_e::SHOTGUN_STATIC});
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
    if(playerComp->m_weaponChange)
    {
        setDisplayWeaponChange(posComp, spriteComp, timerComp, playerComp);
    }
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
            timerComp->m_clock;
    if(playerComp->m_timerShootActive && elapsed_seconds.count() > 0.2)
    {
        setWeaponSprite(playerComp->m_weaponEntity,
                        m_weaponSpriteAssociated[playerComp->m_currentWeapon]);
        playerComp->m_timerShootActive = false;
        playerComp->m_playerShoot = false;
    }
    else if(playerComp->m_playerShoot)
    {
        timerComp->m_clock = std::chrono::system_clock::now();
        WeaponsSpriteType_e spriteNum = static_cast<WeaponsSpriteType_e>(
                    static_cast<uint32_t>(m_weaponSpriteAssociated
                                          [playerComp->m_currentWeapon]) + 1);

        setWeaponSprite(playerComp->m_weaponEntity, spriteNum);
    }
    m_weaponVertice.clear();
    m_weaponVertice.loadVertexStandartTextureComponent(*posComp, *spriteComp);
}

//===================================================================
void StaticDisplaySystem::setDisplayWeaponChange(PositionVertexComponent *posComp,
                                                 SpriteTextureComponent *spriteComp,
                                                 TimerComponent *timerComp,
                                                 PlayerConfComponent *playerComp)
{
    assert(playerComp);
    assert(timerComp);
    assert(posComp);
    assert(spriteComp);
    //move down
    if(playerComp->m_previousWeapon != playerComp->m_currentWeapon)
    {
        modVertexPosVertical(posComp, -m_speedMoveWeaponChange);
        if(posComp->m_vertex[0].second <= -1.0f)
        {
            playerComp->m_previousWeapon = playerComp->m_currentWeapon;
            setWeaponSprite(playerComp->m_weaponEntity,
                            m_weaponSpriteAssociated[playerComp->m_currentWeapon]);
            float diffVert = posComp->m_vertex[3].second - posComp->m_vertex[0].second;
            modVertexPosVertical(posComp, diffVert);
        }
    }
    //move up
    else
    {
        MemPositionsVertexComponents *memPosComp = stairwayToComponentManager().
                searchComponentByType<MemPositionsVertexComponents>(playerComp->m_weaponEntity,
                                                                    Components_e::MEM_POSITIONS_VERTEX_COMPONENT);
        assert(memPosComp);
        modVertexPosVertical(posComp, m_speedMoveWeaponChange);
        uint32_t index = static_cast<uint32_t>(playerComp->m_currentWeapon);
        if(posComp->m_vertex[0].second >= memPosComp->m_vectSpriteData[index][0].second)
        {
            posComp->m_vertex[0].second = memPosComp->m_vectSpriteData[index][0].second;
            playerComp->m_weaponChange = false;
        }
    }
}

//===================================================================
void modVertexPosVertical(PositionVertexComponent *posComp, float mod)
{
    for(uint32_t i = 0; i < posComp->m_vertex.size(); ++i)
    {
        posComp->m_vertex[i].second += mod;
    }
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
void StaticDisplaySystem::setWeaponSprite(uint32_t weaponEntity, WeaponsSpriteType_e weaponSprite)
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
