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
class MainEngine;

using ArrayControlKey_t = std::array<uint32_t, static_cast<uint32_t>(ControlKey_e::TOTAL)>;

enum class InputType_e
{
    KEYBOARD,
    GAMEPAD_BUTTONS,
    GAMEPAD_AXIS
};

enum class VertexID_e
{
    WEAPON,
    LIFE_WRITE,
    AMMO_WRITE,
    MENU_WRITE,
    MENU_CURSOR,
    INFO,
    RESOLUTION_DISPLAY_MENU,
    FULLSCREEN,
    INPUT,
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
    void setWeaponSprite(uint32_t weaponEntity, uint32_t weaponNumSprite);
    void memDisplayMenuEntities(uint32_t numMenuResolutionWrite, uint32_t numFullscreenMenuEntity);
    void updateDisplayMenuResolution(const std::string &str);
    void updateMenuEntryFullscreen(bool displayMenufullscreenMode);
    std::string getKeyboardStringKeyAssociated(uint32_t key)const;
    std::string getGamepadStringKeyButtonAssociated(uint32_t key)const;
    std::string getGamepadStringKeyAxisAssociated(uint32_t key, bool axisSense)const;
    void updateNewInputKey(ControlKey_e currentSelectedKey, uint32_t glKey, InputType_e inputType, bool axisSense = false);
    void updateStringWriteEntitiesInputMenu(bool keyboardInputMenuMode, bool defaultInput = true);
    inline void linkMainEngine(MainEngine *mainEngine)
    {
        m_mainEngine = mainEngine;
    }
    inline void setVectTextures(std::vector<Texture> &vectTexture)
    {
        m_ptrVectTexture = &vectTexture;
    }
    inline void memFontDataPtr(FontData const *fontData)
    {
        m_fontDataPtr = fontData;
    }
    inline void memInputMenuEntities(const ArrayControlKey_t &memKeyboardEntities, const ArrayControlKey_t &memGamepadEntities)
    {
        m_inputMenuKeyboardWriteKeysEntities = memKeyboardEntities;
        m_inputMenuGamepadWriteKeysEntities = memGamepadEntities;
    }
private:
    void fillCursorMenuVertex(PlayerConfComponent *playerComp);
    void updateMenuCursorPosition(PlayerConfComponent *playerComp);
    void confWriteVertex(WriteComponent *writeComp, PositionVertexComponent *posComp,
                         VertexID_e type);
    void drawVertex(uint32_t numTexture, VertexID_e type);
    void drawWriteVertex(uint32_t numEntity, VertexID_e type, const std::string &value = "");
    void drawLineWriteVertex(PositionVertexComponent *posComp, WriteComponent *writeComp);
    void confWeaponsVertexFromComponent(PlayerConfComponent *playerComp, SpriteTextureComponent *weaponSpriteComp);
    void treatWeaponShootAnimation(float elapsedSeconds, PlayerConfComponent *playerComp,
                                   TimerComponent *timerComp);
    void setDisplayWeaponChange(PositionVertexComponent *posComp, PlayerConfComponent *playerComp,
                                MemPositionsVertexComponents *memPosComp);
    void setWeaponMovement(PlayerConfComponent *playerComp, PositionVertexComponent *posComp,
                           MemPositionsVertexComponents *memPosComp);
private:
    MainEngine *m_mainEngine;
    FontData const *m_fontDataPtr;
    bool m_cursorInit = false;
    Shader *m_shader;
    std::array<VerticesData, static_cast<uint32_t>(VertexID_e::TOTAL)> m_vertices;
    std::vector<Texture> *m_ptrVectTexture = nullptr;
    float m_speedMoveWeaponChange = 0.05f;
    PairFloat_t m_forkWeaponMovementX = {-0.33f, -0.1f}, m_forkWeaponMovementY = {-0.99f, -0.4f};
    float m_diffTotalDistanceMoveWeaponX = std::abs(m_forkWeaponMovementX.first -
                                                    m_forkWeaponMovementX.second),
    m_halfDiffTotalDistanceMoveWeaponX = m_diffTotalDistanceMoveWeaponX / 2.0f;
    float m_middleWeaponMovementX = m_forkWeaponMovementX.first + (m_forkWeaponMovementX.second -
                                                                   m_forkWeaponMovementX.first) / 2.0f;
    //FORCE UPDATE AT LAUNCH
    uint32_t m_currentCursorPos = static_cast<uint32_t>(MainMenuCursorPos_e::TOTAL), m_resolutionDisplayMenuEntity,
    m_fullscreenMenuEntity;
    ArrayControlKey_t m_inputMenuKeyboardWriteKeysEntities, m_inputMenuGamepadWriteKeysEntities;
};

std::string treatInfoMessageEndLine(const std::string &str, uint32_t lineSize = 20);
void modVertexPos(PositionVertexComponent *posComp, const PairFloat_t &mod);
