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
        writeWeaponsVertexFromComponent(mVectNumEntity[i]);
        drawVertex();
    }
}

//===================================================================
void StaticDisplaySystem::writeWeaponsVertexFromComponent(uint32_t numObserverEntity)
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
    MemPositionsVertexComponents *memPosComp = stairwayToComponentManager().
            searchComponentByType<MemPositionsVertexComponents>(playerComp->m_weaponEntity,
                                                                Components_e::MEM_POSITIONS_VERTEX_COMPONENT);
    assert(memPosComp);
    assert(timerComp);
    assert(posComp);
    assert(spriteComp);
    assert(!posComp->m_vertex.empty());
    if(playerComp->m_weaponChange)
    {
        setDisplayWeaponChange(posComp, playerComp, memPosComp);
    }
    else
    {
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
                timerComp->m_clock;
        if(playerComp->m_playerShoot)
        {
            timerComp->m_clock = std::chrono::system_clock::now();
            WeaponsSpriteType_e spriteNum = static_cast<WeaponsSpriteType_e>(
                        static_cast<uint32_t>(m_weaponSpriteAssociated
                                              [playerComp->m_currentWeapon]) + 1);
            setWeaponSprite(playerComp->m_weaponEntity, spriteNum);
            playerComp->m_playerShoot = false;
            playerComp->m_timerShootActive = true;
        }
        else if(playerComp->m_timerShootActive && elapsed_seconds.count() > 0.2)
        {
            setWeaponSprite(playerComp->m_weaponEntity,
                            m_weaponSpriteAssociated[playerComp->m_currentWeapon]);
            playerComp->m_timerShootActive = false;
        }
        else
        {
            setWeaponMovement(playerComp, posComp, memPosComp);
        }
    }
    m_weaponVertice.clear();
    m_weaponVertice.loadVertexStandartTextureComponent(*posComp, *spriteComp);
}

//===================================================================
void StaticDisplaySystem::setWeaponMovement(PlayerConfComponent *playerComp,
                                            PositionVertexComponent *posComp,
                                            MemPositionsVertexComponents *memPosComp)
{
    if(playerComp->m_inMovement)
    {
        modVertexPos(posComp, playerComp->m_currentMove);
        //check X var
        if(playerComp->m_currentMove.first < EPSILON_FLOAT &&
                posComp->m_vertex[0].first <= m_forkWeaponMovementX.first)
        {
            playerComp->m_currentMove.first *= -1.0f;
        }
        else if(playerComp->m_currentMove.first > EPSILON_FLOAT &&
                posComp->m_vertex[0].first >= m_forkWeaponMovementX.second)
        {
            playerComp->m_currentMove.first *= -1.0f;
        }
        //check Y var
        if(playerComp->m_currentMove.second < EPSILON_FLOAT &&
                posComp->m_vertex[0].second <= m_forkWeaponMovementY.first)
        {
            playerComp->m_currentMove.second *= -1.0f;
        }
        else if(playerComp->m_currentMove.second > EPSILON_FLOAT &&
                posComp->m_vertex[0].second >= m_forkWeaponMovementY.second)
        {
            playerComp->m_currentMove.second *= -1.0f;
        }
        playerComp->m_spritePositionCorrected = true;
    }
    else if(playerComp->m_spritePositionCorrected)
    {
        uint32_t index = static_cast<uint32_t>(m_weaponSpriteAssociated[playerComp->m_currentWeapon]);
        float modX;
        bool change = false;
        if(posComp->m_vertex[0].first < memPosComp->m_vectSpriteData[index][0].first)
        {
            change = true;
            modX = m_speedMoveWeaponChange;
        }
        else if(posComp->m_vertex[0].first > memPosComp->m_vectSpriteData[index][0].first)
        {
            change = true;
            modX = -m_speedMoveWeaponChange;
        }
        if(change)
        {
            modVertexPos(posComp, {modX, m_speedMoveWeaponChange});
            if(posComp->m_vertex[0].second > memPosComp->m_vectSpriteData[index][0].second)
            {
                for(uint32_t i = 0; i < 4; ++i)
                {
                    posComp->m_vertex[i] = memPosComp->m_vectSpriteData[index][i];
                    playerComp->m_spritePositionCorrected = false;
                }
            }
        }
    }
}

//===================================================================
void StaticDisplaySystem::setDisplayWeaponChange(PositionVertexComponent *posComp,
                                                 PlayerConfComponent *playerComp,
                                                 MemPositionsVertexComponents *memPosComp)
{
    //move down
    if(playerComp->m_previousWeapon != playerComp->m_currentWeapon)
    {
        modVertexPos(posComp, {EPSILON_FLOAT, -m_speedMoveWeaponChange});
        if(posComp->m_vertex[0].second <= -1.0f)
        {
            playerComp->m_previousWeapon = playerComp->m_currentWeapon;
            setWeaponSprite(playerComp->m_weaponEntity,
                            m_weaponSpriteAssociated[playerComp->m_currentWeapon]);
            float diffVert = posComp->m_vertex[3].second - posComp->m_vertex[0].second;
            modVertexPos(posComp, {EPSILON_FLOAT, diffVert});
        }
    }
    //move up
    else
    {
        modVertexPos(posComp, {EPSILON_FLOAT, m_speedMoveWeaponChange});
        uint32_t index = static_cast<uint32_t>(playerComp->m_currentWeapon);
        if(posComp->m_vertex[0].second >= memPosComp->m_vectSpriteData[index][0].second)
        {
            posComp->m_vertex[0].second = memPosComp->m_vectSpriteData[index][0].second;
            playerComp->m_weaponChange = false;
        }
    }
}

//===================================================================
void modVertexPos(PositionVertexComponent *posComp, const pairFloat_t &mod)
{
    for(uint32_t i = 0; i < posComp->m_vertex.size(); ++i)
    {
        posComp->m_vertex[i].first += mod.first;
        posComp->m_vertex[i].second += mod.second;
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
