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

enum class VertexID_e
{
    WEAPON,
    LIFE_WRITE,
    AMMO_WRITE,
    MENU_WRITE,
    MENU_CURSOR,
    INFO,
    RESOLUTION_DISPLAY_MENU,
    QUALITY_DISPLAY_MENU,
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
    void memDisplayMenuEntities(uint32_t numMenuResolutionWrite, uint32_t numMenuQualityWrite, uint32_t numFullscreenMenuEntity);
    void updateDisplayMenuResolution(const std::string &str);
    void updateDisplayMenuQuality(const std::string &str);
    void updateMenuEntryFullscreen(bool displayMenufullscreenMode);
    std::string getKeyboardStringKeyAssociated(uint32_t key)const;
    std::string getGamepadStringKeyButtonAssociated(uint32_t key)const;
    std::string getGamepadStringKeyAxesAssociated(uint32_t key, bool axesSense)const;
    void updateNewInputKey(ControlKey_e currentSelectedKey, uint32_t glKey);
    void updateStringWriteEntitiesInputMenu(bool keyboardInputMenuMode);
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
    inline const std::map<uint32_t, std::string> &getKeyboardInputKeys()const
    {
        return m_inputKeyboardKeyString;
    }
    inline const std::map<uint32_t, std::string> &getGamepadInputKeys()const
    {
        return m_inputGamepadSimpleButtonKeyString;
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
    PairFloat_t m_forkWeaponMovementX = {-0.4f, 0.1f}, m_forkWeaponMovementY = {-0.8f, -0.6f};
    float m_diffTotalDistanceMoveWeaponX = std::abs(m_forkWeaponMovementX.first -
                                                    m_forkWeaponMovementX.second);
    float m_middleWeaponMovementX = m_forkWeaponMovementX.first + (m_forkWeaponMovementX.second -
                                                                   m_forkWeaponMovementX.first) / 2.0f;
    //FORCE UPDATE AT LAUNCH
    uint32_t m_currentCursorPos = static_cast<uint32_t>(MainMenuCursorPos_e::TOTAL), m_resolutionDisplayMenuEntity, m_qualityMenuEntity,
    m_fullscreenMenuEntity;
    ArrayControlKey_t m_inputMenuKeyboardWriteKeysEntities, m_inputMenuGamepadWriteKeysEntities;
    const std::map<uint32_t, std::string> m_inputKeyboardKeyString = {
        {GLFW_KEY_SPACE, "SPACE"},
        {GLFW_KEY_APOSTROPHE, "APOSTROPHE"},
        {GLFW_KEY_COMMA, "COMMA"},
        {GLFW_KEY_MINUS, "MINUS"},
        {GLFW_KEY_PERIOD, "PERIOD"},
        {GLFW_KEY_SLASH, "SLASH"},
        {GLFW_KEY_0, "0"},
        {GLFW_KEY_1, "1"},
        {GLFW_KEY_2, "2"},
        {GLFW_KEY_3, "3"},
        {GLFW_KEY_4, "4"},
        {GLFW_KEY_5, "5"},
        {GLFW_KEY_6, "6"},
        {GLFW_KEY_7, "7"},
        {GLFW_KEY_8, "8"},
        {GLFW_KEY_9, "9"},
        {GLFW_KEY_SEMICOLON, "SEMICOLON"},
        {GLFW_KEY_EQUAL, "EQUAL"},
        {GLFW_KEY_A, "A"},
        {GLFW_KEY_B, "B"},
        {GLFW_KEY_C, "C"},
        {GLFW_KEY_D, "D"},
        {GLFW_KEY_E, "E"},
        {GLFW_KEY_F, "F"},
        {GLFW_KEY_G, "G"},
        {GLFW_KEY_H, "H"},
        {GLFW_KEY_I, "I"},
        {GLFW_KEY_J, "J"},
        {GLFW_KEY_K, "K"},
        {GLFW_KEY_L, "L"},
        {GLFW_KEY_M, "M"},
        {GLFW_KEY_N, "N"},
        {GLFW_KEY_O, "O"},
        {GLFW_KEY_P, "P"},
        {GLFW_KEY_Q, "Q"},
        {GLFW_KEY_R, "R"},
        {GLFW_KEY_S, "S"},
        {GLFW_KEY_T, "T"},
        {GLFW_KEY_U, "U"},
        {GLFW_KEY_V, "V"},
        {GLFW_KEY_W, "W"},
        {GLFW_KEY_X, "X"},
        {GLFW_KEY_Y, "Y"},
        {GLFW_KEY_Z, "Z"},
        {GLFW_KEY_LEFT_BRACKET, "LEFT BRACKET"},
        {GLFW_KEY_BACKSLASH, "BACKSLASH"},
        {GLFW_KEY_RIGHT_BRACKET, "RIGHT BRACKET"},
        {GLFW_KEY_GRAVE_ACCENT, "GRAVE ACCENT"},
        {GLFW_KEY_INSERT, "INSERT"},
        {GLFW_KEY_RIGHT, "RIGHT"},
        {GLFW_KEY_LEFT, "LEFT"},
        {GLFW_KEY_DOWN, "DOWN"},
        {GLFW_KEY_UP, "UP"},
        {GLFW_KEY_PAGE_UP, "PAGE UP"},
        {GLFW_KEY_PAGE_DOWN, "PAGE DOWN"},
        {GLFW_KEY_HOME, "HOME"},
        {GLFW_KEY_END, "END"},
        {GLFW_KEY_KP_0, "KP 0"},
        {GLFW_KEY_KP_1, "KP 1"},
        {GLFW_KEY_KP_2, "KP 2"},
        {GLFW_KEY_KP_3, "KP 3"},
        {GLFW_KEY_KP_4, "KP 4"},
        {GLFW_KEY_KP_5, "KP 5"},
        {GLFW_KEY_KP_6, "KP 6"},
        {GLFW_KEY_KP_7, "KP 7"},
        {GLFW_KEY_KP_8, "KP 8"},
        {GLFW_KEY_KP_9, "KP 9"},
        {GLFW_KEY_KP_DECIMAL, "KP DECIMAL"},
        {GLFW_KEY_KP_DIVIDE, "KP DIVIDE"},
        {GLFW_KEY_KP_MULTIPLY, "KP MULTIPLY"},
        {GLFW_KEY_KP_SUBTRACT, "KP SUBTRACT"},
        {GLFW_KEY_KP_ADD, "KP ADD"},
        {GLFW_KEY_KP_EQUAL, "KP EQUAL"},
        {GLFW_KEY_LEFT_SHIFT, "LEFT SHIFT"},
        {GLFW_KEY_LEFT_CONTROL, "LEFT CONTROL"},
        {GLFW_KEY_LEFT_ALT, "LEFT ALT"},
        {GLFW_KEY_LEFT_SUPER, "LEFT SUPER"},
        {GLFW_KEY_RIGHT_SHIFT, "RIGHT SHIFT"},
        {GLFW_KEY_RIGHT_CONTROL, "RIGHT CONTROL"},
        {GLFW_KEY_RIGHT_ALT, "RIGHT ALT"},
        {GLFW_KEY_RIGHT_SUPER, "RIGHT SUPER"},
        {GLFW_KEY_MENU, "MENU"}
    },
    m_inputGamepadSimpleButtonKeyString = {
        {GLFW_GAMEPAD_BUTTON_A, "BUTTON A"},
        {GLFW_GAMEPAD_BUTTON_B, "BUTTON B"},
        {GLFW_GAMEPAD_BUTTON_X, "BUTTON X"},
        {GLFW_GAMEPAD_BUTTON_Y, "BUTTON Y"},
        {GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "LEFT BUMPER"},
        {GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "RIGHT BUMPER"},
        {GLFW_GAMEPAD_BUTTON_BACK, "BACK"},
        {GLFW_GAMEPAD_BUTTON_START, "START"},
        {GLFW_GAMEPAD_BUTTON_GUIDE, "GUIDE"},
        {GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "LEFT THUMB"},
        {GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "RIGHT THUMB"},
        {GLFW_GAMEPAD_BUTTON_DPAD_UP, "DPAD UP"},
        {GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "DPAD RIGHT"},
        {GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "DPAD DOWN"},
        {GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "DPAD LEFT"},
    },
    m_inputGamepadAxesKeyString = {
        {GLFW_GAMEPAD_AXIS_LEFT_X, "AXIS LEFT X"},
        {GLFW_GAMEPAD_AXIS_LEFT_Y, "AXIS LEFT Y"},
        {GLFW_GAMEPAD_AXIS_RIGHT_X, "AXIS RIGHT X"},
        {GLFW_GAMEPAD_AXIS_RIGHT_Y, "AXIS RIGHT Y"},
        {GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, "AXIS LEFT TRIGGER"},
        {GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, "AXIS RIGHT TRIGGER"}
    };
};

void modVertexPos(PositionVertexComponent *posComp, const PairFloat_t &mod);
