#include "StaticDisplaySystem.hpp"
#include <ECS/Components/MemPositionsVertexComponents.hpp>
#include <ECS/Components/MemSpriteDataComponent.hpp>
#include <ECS/Components/SpriteTextureComponent.hpp>
#include <ECS/Components/PositionVertexComponent.hpp>
#include <ECS/Components/PlayerConfComponent.hpp>
#include <ECS/Components/TimerComponent.hpp>
#include <ECS/Components/WriteComponent.hpp>
#include <cassert>


//===================================================================
StaticDisplaySystem::StaticDisplaySystem()
{
    bAddComponentToSystem(Components_e::VISION_COMPONENT);
}

//===================================================================
void StaticDisplaySystem::fillCursorMenuVertex(PlayerConfComponent *playerComp)
{
    uint32_t index = static_cast<uint32_t>(VertexID_e::MENU_CURSOR);
    SpriteTextureComponent *spriteComp = stairwayToComponentManager().
            searchComponentByType<SpriteTextureComponent>(playerComp->m_menuCursorEntity,
                                                          Components_e::SPRITE_TEXTURE_COMPONENT);
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(playerComp->m_menuCursorEntity,
                                                          Components_e::POSITION_VERTEX_COMPONENT);
    assert(spriteComp);
    assert(posComp);
    m_vertices[index].clear();
    m_vertices[index].loadVertexStandartTextureComponent(*posComp, *spriteComp);
    m_cursorInit = true;
}

//===================================================================
void StaticDisplaySystem::updateMenuCursorPosition(PlayerConfComponent *playerComp)
{
    PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(playerComp->m_menuCursorEntity,
                                                               Components_e::POSITION_VERTEX_COMPONENT);
    assert(posComp);
    WriteComponent *writeMenuComp = stairwayToComponentManager().
                searchComponentByType<WriteComponent>(playerComp->m_menuEntity,
                                                               Components_e::WRITE_COMPONENT);
    assert(writeMenuComp);
    float upPos = writeMenuComp->m_upLeftPositionGL.second -
            static_cast<float>(playerComp->m_currentCursorPos) * writeMenuComp->m_fontSize,
            downPos = upPos - writeMenuComp->m_fontSize * 2.0f;
    posComp->m_vertex[0].second = upPos;
    posComp->m_vertex[1].second = upPos;
    posComp->m_vertex[2].second = downPos;
    posComp->m_vertex[3].second = downPos;
    m_currentCursorPos = playerComp->m_currentCursorPos;
    m_cursorInit = false;
}

//===================================================================
void StaticDisplaySystem::execSystem()
{
    System::execSystem();
    m_shader->use();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        PlayerConfComponent *playerComp = stairwayToComponentManager().
                    searchComponentByType<PlayerConfComponent>(mVectNumEntity[i],
                                                               Components_e::PLAYER_CONF_COMPONENT);
        assert(playerComp);
        //DRAW WEAPON
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(playerComp->m_weaponEntity,
                                                              Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        confWeaponsVertexFromComponent(playerComp, spriteComp);
        drawVertex(spriteComp->m_spriteData->m_textureNum, VertexID_e::WEAPON);
        std::string strAmmoDisplay = STR_PLAYER_AMMO + std::to_string(playerComp->m_ammunationsCount[static_cast<uint32_t>(playerComp->m_currentWeapon)]);
        treatWriteVertex(playerComp->m_ammoWriteEntity, VertexID_e::AMMO_WRITE, strAmmoDisplay);
        treatWriteVertex(playerComp->m_lifeWriteEntity, VertexID_e::LIFE_WRITE, STR_PLAYER_LIFE +
                         std::to_string(playerComp->m_life));
    }
}

//===================================================================
void StaticDisplaySystem::displayMenu()
{
    System::execSystem();
    m_shader->use();
    for(uint32_t i = 0; i < mVectNumEntity.size(); ++i)
    {
        PlayerConfComponent *playerComp = stairwayToComponentManager().
                    searchComponentByType<PlayerConfComponent>(mVectNumEntity[i],
                                                               Components_e::PLAYER_CONF_COMPONENT);
        assert(playerComp);
        treatWriteVertex(playerComp->m_menuEntity, VertexID_e::MENU_WRITE);
        SpriteTextureComponent *spriteComp = stairwayToComponentManager().
                searchComponentByType<SpriteTextureComponent>(playerComp->m_menuCursorEntity,
                                                              Components_e::SPRITE_TEXTURE_COMPONENT);
        assert(spriteComp);
        if(m_currentCursorPos != playerComp->m_currentCursorPos)
        {
            updateMenuCursorPosition(playerComp);
        }
        if(!m_cursorInit)
        {
            fillCursorMenuVertex(playerComp);
        }
        drawVertex(spriteComp->m_spriteData->m_textureNum, VertexID_e::MENU_CURSOR);
    }
}

//===================================================================
void StaticDisplaySystem::confWriteVertex(WriteComponent *writeComp,
                                          PositionVertexComponent *posComp,
                                          VertexID_e type)
{
    uint32_t index = static_cast<uint32_t>(type);
    assert(!writeComp->m_fontSpriteData.empty());
    m_vertices[index].clear();
    drawLineWriteVertex(posComp, writeComp);
    m_vertices[index].loadVertexWriteTextureComponent(*posComp, *writeComp);
}

//===================================================================
void StaticDisplaySystem::drawVertex(uint32_t numTexture, VertexID_e type)
{
    assert(static_cast<size_t>(numTexture) < m_ptrVectTexture->size());
    m_ptrVectTexture->operator[](numTexture).bind();
    uint32_t index = static_cast<uint32_t>(type);
    m_vertices[index].confVertexBuffer();
    m_vertices[index].drawElement();
}

//===================================================================
void StaticDisplaySystem::treatWriteVertex(uint32_t numEntity, VertexID_e type,
                                           const std::string &value)
{
    WriteComponent *writeComp = stairwayToComponentManager().
                searchComponentByType<WriteComponent>(numEntity,
                                                      Components_e::WRITE_COMPONENT);
    PositionVertexComponent *posComp = stairwayToComponentManager().
                searchComponentByType<PositionVertexComponent>(numEntity,
                                                               Components_e::POSITION_VERTEX_COMPONENT);
    assert(writeComp);
    assert(posComp);
    if(!value.empty())
    {
        if(value != writeComp->m_str)
        {
            writeComp->m_fontSpriteData = m_fontDataPtr->getWriteData(value,
                                                                      writeComp->m_numTexture);
        }
        writeComp->m_str = value;
    }
    confWriteVertex(writeComp, posComp, type);
    drawVertex(writeComp->m_numTexture, type);
}

//===================================================================
void StaticDisplaySystem::confWeaponsVertexFromComponent(PlayerConfComponent *playerComp,
                                                         SpriteTextureComponent *weaponSpriteComp)
{
    PositionVertexComponent *posComp = stairwayToComponentManager().
            searchComponentByType<PositionVertexComponent>(playerComp->m_weaponEntity,
                                                           Components_e::POSITION_VERTEX_COMPONENT);
    TimerComponent *timerComp = stairwayToComponentManager().
            searchComponentByType<TimerComponent>(playerComp->m_weaponEntity,
                                                  Components_e::TIMER_COMPONENT);
    MemPositionsVertexComponents *memPosComp = stairwayToComponentManager().
            searchComponentByType<MemPositionsVertexComponents>(playerComp->m_weaponEntity,
                                                                Components_e::MEM_POSITIONS_VERTEX_COMPONENT);
    assert(memPosComp);
    assert(timerComp);
    assert(posComp);
    assert(!posComp->m_vertex.empty());
    if(playerComp->m_weaponChange)
    {
        setDisplayWeaponChange(posComp, playerComp, memPosComp);
    }
    else
    {
        if(playerComp->m_playerShoot)
        {
            timerComp->m_clockA = std::chrono::system_clock::now();
            playerComp->m_numWeaponSprite =
                    m_weaponSpriteAssociated[playerComp->m_currentWeapon] + 1;
            WeaponsSpriteType_e spriteNum = static_cast<WeaponsSpriteType_e>(
                        playerComp->m_numWeaponSprite);
            setWeaponSprite(playerComp->m_weaponEntity, spriteNum);
            playerComp->m_playerShoot = false;
            playerComp->m_timerShootActive = true;
            playerComp->m_shootFirstPhase = true;
        }
        else if(playerComp->m_timerShootActive)
        {
            std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() -
                    timerComp->m_clockA;
            treatWeaponShootAnimation(elapsed_seconds.count(),
                                      playerComp, timerComp);
        }
        else if(!playerComp->m_timerShootActive)
        {
            setWeaponMovement(playerComp, posComp, memPosComp);
        }
    }
    uint32_t index = static_cast<uint32_t>(VertexID_e::WEAPON);
    m_vertices[index].clear();
    m_vertices[index].loadVertexStandartTextureComponent(*posComp, *weaponSpriteComp);
}

//===================================================================
void StaticDisplaySystem::treatWeaponShootAnimation(float elapsedSeconds,
                                                    PlayerConfComponent *playerComp,
                                                    TimerComponent *timerComp)
{
    WeaponsSpriteType_e spriteNumMax, spriteNumBase, spriteNumLastAnim;
    if(weapon == WeaponsType_e::SHOTGUN)
    {
        spriteNumMax = WeaponsSpriteType_e::SHOTGUN_RELOAD_C;
        spriteNumBase = WeaponsSpriteType_e::SHOTGUN_STATIC;
        spriteNumLastAnim = WeaponsSpriteType_e::SHOTGUN_RELOAD_A;
    }
    else if(weapon == WeaponsType_e::FIST)
    {
        spriteNumMax = WeaponsSpriteType_e::FIST_ATTACK_C;
        spriteNumBase = WeaponsSpriteType_e::FIST_STATIC;
        spriteNumLastAnim = WeaponsSpriteType_e::FIST_ATTACK_A;
    }
    else if(weapon == WeaponsType_e::PLASMA_RIFLE)
    {
        spriteNumMax = WeaponsSpriteType_e::PLASMA_RIFLE_RELOAD;
        spriteNumBase = WeaponsSpriteType_e::PLASMA_RIFLE_STATIC;
    }
    //remove warning GUN
    else
    {
        spriteNumMax = WeaponsSpriteType_e::GUN_ATTACK_B;
        spriteNumBase = WeaponsSpriteType_e::GUN_STATIC;
    }
    WeaponsSpriteType_e spriteNum = static_cast<WeaponsSpriteType_e>(
                playerComp->m_numWeaponSprite);
    if(elapsedSeconds > m_weaponsLatences[static_cast<uint32_t>(weapon)])
    {
        if(playerComp->m_shootFirstPhase)
        {
            if(spriteNum == spriteNumMax)
            {
                if(weapon == WeaponsType_e::GUN || weapon == WeaponsType_e::PLASMA_RIFLE)
                {
                    playerComp->m_timerShootActive = false;
                    playerComp->m_numWeaponSprite = static_cast<uint32_t>(spriteNumBase);
                }
                else
                {
                    playerComp->m_shootFirstPhase = false;
                    --playerComp->m_numWeaponSprite;
                }
            }
            else
            {
                ++playerComp->m_numWeaponSprite;
            }
        }
        else
        {
            if(spriteNum == spriteNumBase)
            {
                playerComp->m_timerShootActive = false;
                return;
            }
            if(spriteNum == spriteNumLastAnim)
            {
                playerComp->m_numWeaponSprite = static_cast<uint32_t>(spriteNumBase);
            }
            else
            {
                --playerComp->m_numWeaponSprite;
            }
        }
        setWeaponSprite(playerComp->m_weaponEntity,
                        static_cast<WeaponsSpriteType_e>(playerComp->m_numWeaponSprite));
        timerComp->m_clockA = std::chrono::system_clock::now();
    }
}


//===================================================================
void StaticDisplaySystem::setWeaponMovement(PlayerConfComponent *playerComp,
                                            PositionVertexComponent *posComp,
                                            MemPositionsVertexComponents *memPosComp)
{
    uint32_t index = m_weaponSpriteAssociated[playerComp->m_currentWeapon];
    if(playerComp->m_inMovement)
    {
        modVertexPos(posComp, playerComp->m_currentWeaponMove);
        //check X var
        if(playerComp->m_currentWeaponMove.first < EPSILON_FLOAT &&
                posComp->m_vertex[0].first <= m_forkWeaponMovementX.first)
        {
            playerComp->m_currentWeaponMove.first *= -1.0f;
            //fix go down issue
            modVertexPos(posComp, {EPSILON_FLOAT,
                                   std::abs(memPosComp->m_vectSpriteData[index][0].second -
                                   posComp->m_vertex[0].second)});
        }
        else if(playerComp->m_currentWeaponMove.first > EPSILON_FLOAT &&
                posComp->m_vertex[0].first >= m_forkWeaponMovementX.second)
        {
            playerComp->m_currentWeaponMove.first *= -1.0f;
            //fix go down issue
            modVertexPos(posComp, {EPSILON_FLOAT,
                                   std::abs(memPosComp->m_vectSpriteData[index][0].second -
                                   posComp->m_vertex[0].second)});
        }
        //check Y var
        if(posComp->m_vertex[2].second >= (-1.0f -
                                           std::abs(playerComp->m_currentWeaponMove.second)))
        {
            modVertexPos(posComp, {EPSILON_FLOAT,
                                   std::abs(playerComp->m_currentWeaponMove.second) * -1.0f});
        }
        if((posComp->m_vertex[0].second < memPosComp->m_vectSpriteData[index][0].second) &&
                ((posComp->m_vertex[0].first < m_middleWeaponMovementX &&
            playerComp->m_currentWeaponMove.first < EPSILON_FLOAT) ||
                (posComp->m_vertex[0].first > m_middleWeaponMovementX &&
                 playerComp->m_currentWeaponMove.first > EPSILON_FLOAT)))
        {
            playerComp->m_currentWeaponMove.second = std::abs(playerComp->m_currentWeaponMove.second);
        }
        else
        {
            playerComp->m_currentWeaponMove.second =
                    std::abs(playerComp->m_currentWeaponMove.second) * (-1.0f);
        }
        playerComp->m_spritePositionCorrected = false;
    }
    //put weapon to standart position
    else if(!playerComp->m_spritePositionCorrected)
    {
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
            uint32_t currentWeapon =
                        m_weaponSpriteAssociated[playerComp->m_currentWeapon];
            if(posComp->m_vertex[0].second >= memPosComp->m_vectSpriteData[currentWeapon][0].second)
            {
                for(uint32_t i = 0; i < 4; ++i)
                {
                    posComp->m_vertex[i] = memPosComp->m_vectSpriteData[currentWeapon][i];
                    playerComp->m_spritePositionCorrected = true;
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
        uint32_t index = m_weaponSpriteAssociated[playerComp->m_currentWeapon];
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
void StaticDisplaySystem::drawLineWriteVertex(PositionVertexComponent *posComp,
                                              WriteComponent *writeComp)
{
    assert(!writeComp->m_fontSpriteData.empty());
    posComp->m_vertex.clear();
    posComp->m_vertex.reserve(writeComp->m_fontSpriteData.size() * 4);
    float currentX = writeComp->m_upLeftPositionGL.first, diffX,
            currentY = writeComp->m_upLeftPositionGL.second, diffY = writeComp->m_fontSize;
    std::array<pairFloat_t, 4> *memArray = &(writeComp->m_fontSpriteData[0].get().m_texturePosVertex);
    float cohef = ((*memArray)[2].second - (*memArray)[0].second) / writeComp->m_fontSize;
    uint32_t cmptSpriteData = 0;
    for(uint32_t i = 0; i < writeComp->m_str.size(); ++i)
    {
        if(writeComp->m_str[i] == ' ')
        {
            currentX += writeComp->m_fontSize / 4.0f;
            continue;
        }
        else if(writeComp->m_str[i] == '\\')
        {
            currentX = writeComp->m_upLeftPositionGL.first;
            currentY -= diffY;
            continue;
        }
        assert(cmptSpriteData < writeComp->m_fontSpriteData.size());
        memArray = &(writeComp->m_fontSpriteData[cmptSpriteData].get().m_texturePosVertex);
        diffX = ((*memArray)[1].first - (*memArray)[0].first) / cohef;
        posComp->m_vertex.emplace_back(pairFloat_t{currentX, currentY});
        posComp->m_vertex.emplace_back(pairFloat_t{currentX + diffX, currentY});
        posComp->m_vertex.emplace_back(pairFloat_t{currentX + diffX, currentY - diffY});
        posComp->m_vertex.emplace_back(pairFloat_t{currentX, currentY - diffY});
        currentX += diffX;
        ++cmptSpriteData;
    }
}

//===================================================================
void StaticDisplaySystem::setShader(Shader &shader)
{
    m_shader = &shader;
}

//===================================================================
void StaticDisplaySystem::setWeaponSprite(uint32_t weaponEntity, uint32_t weaponNumSprite)
{
    m_currentWeaponSprite = weaponNumSprite;
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
    uint32_t index = static_cast<uint32_t>(weaponNumSprite);
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
