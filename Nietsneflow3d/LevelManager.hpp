#pragma once

#include <Level.hpp>
#include <PictureData.hpp>
#include <FontData.hpp>
#include <ECS/Systems/StaticDisplaySystem.hpp>
#include <ECS/Systems/InputSystem.hpp>
#include <set>
#include <iniwriter.h>
#include <fstream>

class INIReader;
struct MemPlayerConf;
using SetStr_t = std::set<std::string>;

struct MemSpriteData
{
    uint8_t m_numSprite;
    PairFloat_t m_GLSize;
};

using MapVisibleShotData_t = std::map<std::string, std::pair<std::string ,std::vector<MemSpriteData>>>;
using PairImpactData_t = std::pair<std::vector<MemSpriteData>, MemSpriteData>;
using MapImpactData_t = std::map<std::string, PairImpactData_t>;
using pairI_t = std::pair<int, int>;

struct WeaponINIData
{
    std::vector<MemSpriteData> m_spritesData;
    uint32_t m_maxAmmo, m_simultaneousShots, m_lastAnimNum, m_order, m_damage;
    AttackType_e m_attackType;
    AnimationMode_e m_animMode;
    std::string m_visibleShootID, m_impactID, m_shotSound, m_reloadSound;
    float m_animationLatency, m_shotVelocity;
    bool m_startingPossess;
    std::optional<float> m_damageCircleRay;
    std::optional<uint32_t> m_startingAmmoCount;
};

struct WallData
{
    std::vector<uint8_t> m_sprites;
    std::set<PairUI_t> m_TileGamePosition, m_removeGamePosition;
};

struct AssociatedTriggerData
{
    PairUI_t m_pos;
    MemSpriteData m_displayData;
};

struct MoveableWallData
{
    std::vector<uint8_t> m_sprites;
    std::set<PairUI_t> m_TileGamePosition, m_removeGamePosition;
    std::vector<std::pair<Direction_e, uint32_t>> m_directionMove;
    TriggerWallMoveType_e m_triggerType;
    TriggerBehaviourType_e m_triggerBehaviourType;
    float m_velocity;
    std::optional<AssociatedTriggerData> m_associatedTriggerData;
    std::optional<PairUI_t> m_groundTriggerPos;
};

struct SettingsData
{
    //AUDIO
    uint32_t m_musicVolume, m_effectsVolume;
    //DISPLAY
    bool m_fullscreen;
    int m_resolutionWidth, m_resolutionHeight;
    //INPUT
    std::array<uint32_t, static_cast<uint32_t>(ControlKey_e::TOTAL)> m_arrayKeyboard;
    std::array<GamepadInputState, static_cast<uint32_t>(ControlKey_e::TOTAL)> m_arrayGamepad;
};

class LevelManager
{
public:
    LevelManager();
    void loadTextureData(const std::string &INIFileName);
    void loadStandardData(const std::string &INIFileName);
    void loadFontData(const std::string &INIFileName);
    bool loadSettingsData();
    void loadLevel(const std::string &INIFileName, uint32_t levelNum);
    void clearExistingPositionsElement();
    void saveAudioSettings(uint32_t musicVolume, uint32_t effectVolume);
    void saveDisplaySettings(const pairI_t &resolution, bool fullscreen);
    void saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                                  const std::map<ControlKey_e, uint32_t> &keyboardArray);
    void saveGameProgress(const MemPlayerConf &playerConf, uint32_t levelNum, std::optional<uint32_t> numSaveFile = {});
    std::optional<std::pair<uint32_t, MemPlayerConf>> loadSavedGame(uint32_t saveNum);
    inline const PictureData &getPictureData()const {return m_pictureData;}
    inline const Level &getLevel()const {return m_level;}
    inline const FontData &getFontData()const {return m_fontData;}
    inline std::string getCursorSpriteName()const {return m_spriteCursorName;}
    inline const std::map<std::string, WallData> getWallData()const{return m_wallData;}
    inline const std::map<std::string, MoveableWallData> getMoveableWallData()const
    {
        return m_mainWallData;
    }
    inline const std::map<std::string, StaticLevelElementData> &getGroundData()const
    {
        return m_groundElement;
    }
    inline const std::map<std::string, StaticLevelElementData> &getCeilingData()const
    {
        return m_ceilingElement;
    }
    inline const std::map<std::string, StaticLevelElementData> &getObjectData()const
    {
        return m_objectElement;
    }
    inline const std::map<std::string, StaticLevelElementData> &getTeleportData()const
    {
        return m_teleportElement;
    }
    inline const StaticLevelElementData &getExitElementData()const
    {
        return m_exitStaticElement;
    }
    inline const std::map<std::string, DoorData> &getDoorData()const
    {
        return m_doorData;
    }
    inline const std::map<std::string, EnemyData> &getEnemiesData()const
    {
        return m_enemyData;
    }
    inline const std::vector<SpriteData> &getPictureSpriteData()const
    {
        return m_pictureData.getSpriteData();
    }
    inline const std::vector<WeaponINIData> &getWeaponsData()const
    {
        return m_vectWeaponsINIData;
    }
    inline const MapImpactData_t &getImpactDisplayData()const
    {
        return m_impactINIData;
    }
    inline const MapVisibleShotData_t &getVisibleShootDisplayData()const
    {
        return m_visibleShootINIData;
    }
    inline const std::vector<MemSpriteData> &getVisibleTeleportData()const
    {
        return m_displayTeleportData;
    }
    inline const BarrelData &getBarrelData()const
    {
        return m_barrelElement;
    }
    inline std::string getPickObjectSoundFile()const
    {
        return m_pickObjectSound;
    }
    inline std::string getDoorOpeningSoundFile()const
    {
        return m_doorOpeningSound;
    }
    inline std::string getHitSoundFile()const
    {
        return m_hitSound;
    }
    inline std::string getTeleportSoundFile()const
    {
        return m_teleportSound;
    }
    inline const SettingsData &getSettingsData()const
    {
        return m_settingsData;
    }
    bool checkSavedGameExists(uint32_t saveNum) const;
private:
    //texture and sprite loading
    void loadTexturePath(const INIReader &reader);
    void loadSpriteData(const INIReader &reader, const std::string &sectionName = "Sprite",
                        bool font = false);
    void loadBackgroundData(const INIReader &reader);
    void loadMusicData(const INIReader &reader);
    //Level element datas loading
    void loadLevelData(const INIReader &reader);
    void loadPositionPlayerData(const INIReader &reader);
    void loadGeneralStaticElements(const INIReader &reader,
                                   LevelStaticElementType_e elementType);
    void loadPositionStaticElements(const INIReader &reader);
    void loadBarrelElements(const INIReader &reader);
    void readStandardStaticElement(const INIReader &reader, StaticLevelElementData &staticElement,
                                   const std::string &sectionName,
                                   LevelStaticElementType_e elementType);
    void fillStandartPositionVect(const INIReader &reader, const std::string &sectionName,
                                  VectPairUI_t &vectPos);
    void fillTeleportPositions(const INIReader &reader, const std::string &sectionName);
    std::optional<PairUI_t> getPosition(const INIReader &reader, const std::string_view sectionName, const std::string_view propertyName);
    bool fillWallPositionVect(const INIReader &reader, const std::string &sectionName, const std::string &propertyName,
                              std::set<PairUI_t> &setPos);
    uint8_t getSpriteId(const INIReader &reader, const std::string &sectionName);
    void loadVisibleShotDisplayData(const INIReader &reader);
    void loadShotImpactDisplayData(const INIReader &reader);
    void loadWeaponsData(const INIReader &reader);
    void loadGeneralSoundData(const INIReader &reader);
    void loadBarrelsData(const INIReader &reader);
    void loadExit(const INIReader &reader);
    void loadVisualTeleportData(const INIReader &reader);
    void loadTriggerElements(const INIReader &reader);
    void loadSpriteData(const INIReader &reader, const std::string &sectionName,
                        StaticLevelElementData &staticElement);
    void loadDisplayData(const INIReader &reader,
                         std::string_view sectionName, std::string_view subSectionName);
    void loadWeaponData(const INIReader &reader,
                        std::string_view sectionName, uint32_t numIt);
    void loadWallData(const INIReader &reader);
    void loadPositionWall(const INIReader &reader);
    void loadTriggerLevelData(const INIReader &reader, const std::string &sectionName);
    void loadDoorData(const INIReader &reader);
    void loadPositionDoorData(const INIReader &reader);
    void loadEnemyData(const INIReader &reader);
    void loadPositionEnemyData(const INIReader &reader);
    void loadUtilsData(const INIReader &reader);
    void loadEnemySprites(const INIReader &reader, const std::string &sectionName,
                          EnemySpriteElementType_e spriteTypeEnum, EnemyData &enemyData);
    void deleteWall(const PairUI_t &coord);
    void loadPositionExit(const INIReader &reader);
    std::vector<uint8_t> getVectSpriteNum(const INIReader &reader, const std::string_view section, const std::string_view param);
    std::vector<PairFloat_t> getVectSpriteGLSize(const INIReader &reader, const std::string_view section, const std::string_view weightParam,
                                                  const std::string_view heightParam);
private:
    uint32_t m_currentSave = 1;
    inipp::Ini<char> m_ini;
    std::ofstream m_outputStream;
    PictureData m_pictureData;
    FontData m_fontData;
    Level m_level;
    std::string m_spriteCursorName;
    StaticLevelElementData m_exitStaticElement;
    std::map<std::string, WallData> m_wallData;
    std::map<std::string, MoveableWallData> m_mainWallData;
    std::map<std::string, uint32_t> m_weaponINIAssociated, m_cardINIAssociated;
    std::map<std::string, StaticLevelElementData> m_groundElement, m_ceilingElement, m_objectElement, m_teleportElement;
    BarrelData m_barrelElement;
    std::map<std::string, DoorData> m_doorData;
    std::map<std::string, EnemyData> m_enemyData;
    std::map<std::string, MemSpriteData> m_triggerDisplayData;
    std::vector<MemSpriteData> m_displayTeleportData;
    //store the sprite number and the screen display size
    std::vector<WeaponINIData> m_vectWeaponsINIData;
    //first moving Shot sprite, all other destruct phase sprites
    MapVisibleShotData_t m_visibleShootINIData;
    MapImpactData_t m_impactINIData;
    SettingsData m_settingsData;
    std::string m_pickObjectSound, m_doorOpeningSound, m_hitSound, m_teleportSound;
    const std::array<std::string, static_cast<uint32_t>(ControlKey_e::TOTAL)> m_inputIDString = {
        "moveForward", "moveBackward", "strafeLeft", "strafeRight", "turnLeft", "turnRight", "action", "shoot", "previousWeapon", "nextWeapon"
    };
    const std::map<std::string, uint32_t> m_inputKeyboardKeyString = {
        {"SPACE", GLFW_KEY_SPACE},
        {"APOSTROPHE", GLFW_KEY_APOSTROPHE},
        {"COMMA", GLFW_KEY_COMMA},
        {"MINUS", GLFW_KEY_MINUS},
        {"PERIOD", GLFW_KEY_PERIOD},
        {"SLASH", GLFW_KEY_SLASH},
        {"0", GLFW_KEY_0},
        {"1", GLFW_KEY_1},
        {"2", GLFW_KEY_2},
        {"3", GLFW_KEY_3},
        {"4", GLFW_KEY_4},
        {"5", GLFW_KEY_5},
        {"6", GLFW_KEY_6},
        {"7", GLFW_KEY_7},
        {"8", GLFW_KEY_8},
        {"9", GLFW_KEY_9},
        {"SEMICOLON", GLFW_KEY_SEMICOLON},
        {"EQUAL", GLFW_KEY_EQUAL},
        {"A", GLFW_KEY_A},
        {"B", GLFW_KEY_B},
        {"C", GLFW_KEY_C},
        {"D", GLFW_KEY_D},
        {"E", GLFW_KEY_E},
        {"F", GLFW_KEY_F},
        {"G", GLFW_KEY_G},
        {"H", GLFW_KEY_H},
        {"I", GLFW_KEY_I},
        {"J", GLFW_KEY_J},
        {"K", GLFW_KEY_K},
        {"L", GLFW_KEY_L},
        {"M", GLFW_KEY_M},
        {"N", GLFW_KEY_N},
        {"O", GLFW_KEY_O},
        {"P", GLFW_KEY_P},
        {"Q", GLFW_KEY_Q},
        {"R", GLFW_KEY_R},
        {"S", GLFW_KEY_S},
        {"T", GLFW_KEY_T},
        {"U", GLFW_KEY_U},
        {"V", GLFW_KEY_V},
        {"W", GLFW_KEY_W},
        {"X", GLFW_KEY_X},
        {"Y", GLFW_KEY_Y},
        {"Z", GLFW_KEY_Z},
        {"LEFT BRACKET", GLFW_KEY_LEFT_BRACKET},
        {"BACKSLASH", GLFW_KEY_BACKSLASH},
        {"RIGHT BRACKET", GLFW_KEY_RIGHT_BRACKET},
        {"GRAVE ACCENT", GLFW_KEY_GRAVE_ACCENT},
        {"INSERT", GLFW_KEY_INSERT},
        {"RIGHT", GLFW_KEY_RIGHT},
        {"LEFT", GLFW_KEY_LEFT},
        {"DOWN", GLFW_KEY_DOWN},
        {"UP", GLFW_KEY_UP},
        {"PAGE UP", GLFW_KEY_PAGE_UP},
        {"PAGE DOWN", GLFW_KEY_PAGE_DOWN},
        {"HOME", GLFW_KEY_HOME},
        {"END", GLFW_KEY_END},
        {"KP 0", GLFW_KEY_KP_0},
        {"KP 1", GLFW_KEY_KP_1},
        {"KP 2", GLFW_KEY_KP_2},
        {"KP 3", GLFW_KEY_KP_3},
        {"KP 4", GLFW_KEY_KP_4},
        {"KP 5", GLFW_KEY_KP_5},
        {"KP 6", GLFW_KEY_KP_6},
        {"KP 7", GLFW_KEY_KP_7},
        {"KP 8", GLFW_KEY_KP_8},
        {"KP 9", GLFW_KEY_KP_9},
        {"KP DECIMAL", GLFW_KEY_KP_DECIMAL},
        {"KP DIVIDE", GLFW_KEY_KP_DIVIDE},
        {"KP MULTIPLY", GLFW_KEY_KP_MULTIPLY},
        {"KP SUBTRACT", GLFW_KEY_KP_SUBTRACT},
        {"KP ADD", GLFW_KEY_KP_ADD},
        {"KP EQUAL", GLFW_KEY_KP_EQUAL},
        {"LEFT SHIFT", GLFW_KEY_LEFT_SHIFT},
        {"LEFT CONTROL", GLFW_KEY_LEFT_CONTROL},
        {"LEFT ALT", GLFW_KEY_LEFT_ALT},
        {"LEFT SUPER", GLFW_KEY_LEFT_SUPER},
        {"RIGHT SHIFT", GLFW_KEY_RIGHT_SHIFT},
        {"RIGHT CONTROL", GLFW_KEY_RIGHT_CONTROL},
        {"RIGHT ALT", GLFW_KEY_RIGHT_ALT},
        {"RIGHT SUPER", GLFW_KEY_RIGHT_SUPER},
        {"MENU", GLFW_KEY_MENU}
    };
    const std::map<std::string, GamepadInputState> m_inputGamepadKeyString = {
        {"ButtonA", {true, GLFW_GAMEPAD_BUTTON_A, {}}},
        {"ButtonB", {true, GLFW_GAMEPAD_BUTTON_B, {}}},
        {"ButtonX", {true, GLFW_GAMEPAD_BUTTON_X, {}}},
        {"ButtonY", {true, GLFW_GAMEPAD_BUTTON_Y, {}}},
        {"ButtonLeftBumper", {true, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, {}}},
        {"ButtonRightBumper", {true, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, {}}},
        {"ButtonBack", {true, GLFW_GAMEPAD_BUTTON_BACK, {}}},
        {"ButtonStart", {true, GLFW_GAMEPAD_BUTTON_START, {}}},
        {"ButtonGuide", {true, GLFW_GAMEPAD_BUTTON_GUIDE, {}}},
        {"ButtonLeftThumb", {true, GLFW_GAMEPAD_BUTTON_LEFT_THUMB, {}}},
        {"ButtonRightThumb", {true, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, {}}},
        {"ButtonUp", {true, GLFW_GAMEPAD_BUTTON_DPAD_UP, {}}},
        {"ButtonDown", {true, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, {}}},
        {"ButtonLeft", {true, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, {}}},
        {"ButtonRight", {true, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, {}}},

        {"AxisLeftY-", {false, GLFW_GAMEPAD_AXIS_LEFT_Y, false}},
        {"AxisLeftY+", {false, GLFW_GAMEPAD_AXIS_LEFT_Y, true}},
        {"AxisLeftX-", {false, GLFW_GAMEPAD_AXIS_LEFT_X, false}},
        {"AxisLeftX+", {false, GLFW_GAMEPAD_AXIS_LEFT_X, true}},
        {"AxisRightY-", {false, GLFW_GAMEPAD_AXIS_RIGHT_Y, false}},
        {"AxisRightY+", {false, GLFW_GAMEPAD_AXIS_RIGHT_Y, true}},
        {"AxisRightX-", {false, GLFW_GAMEPAD_AXIS_RIGHT_X, false}},
        {"AxisRightX+", {false, GLFW_GAMEPAD_AXIS_RIGHT_X, true}},
        {"AxisLeftTrigger+", {false, GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, true}},
        {"AxisLeftTrigger-", {false, GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, false}},
        {"AxisRightTrigger+", {false, GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, true}},
        {"AxisRightTrigger-", {false, GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, false}}
    };

    const std::map<uint32_t, std::string> m_inputGamepadSimpleButtons = {
        {GLFW_GAMEPAD_BUTTON_A, "ButtonA"},
        {GLFW_GAMEPAD_BUTTON_B, "ButtonB"},
        {GLFW_GAMEPAD_BUTTON_X, "ButtonX"},
        {GLFW_GAMEPAD_BUTTON_Y, "ButtonY"},
        {GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "ButtonLeftBumper"},
        {GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "ButtonRightBumper"},
        {GLFW_GAMEPAD_BUTTON_BACK, "ButtonBack"},
        {GLFW_GAMEPAD_BUTTON_START, "ButtonStart"},
        {GLFW_GAMEPAD_BUTTON_GUIDE, "ButtonGuide"},
        {GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "ButtonLeftThumb"},
        {GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "ButtonRightThumb"},
        {GLFW_GAMEPAD_BUTTON_DPAD_UP, "ButtonUp"},
        {GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "ButtonRight"},
        {GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "ButtonDown"},
        {GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "ButtonLeft"},
    };

    //!!!WARNING!!!
    // GLFW_GAMEPAD_AXIS_LEFT_X --> OK
    // GLFW_GAMEPAD_AXIS_LEFT_Y --> POS & NEG REVERSED
    // GLFW_GAMEPAD_AXIS_LEFT_TRIGGER --> GLFW_GAMEPAD_AXIS_RIGHT_Y
    // GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER --> OK
    // GLFW_GAMEPAD_AXIS_RIGHT_X --> GLFW_GAMEPAD_AXIS_LEFT_TRIGGER
    // GLFW_GAMEPAD_AXIS_RIGHT_Y --> GLFW_GAMEPAD_AXIS_RIGHT_X
    const std::map<uint32_t, std::string> m_inputGamepadAxis = {
        {GLFW_GAMEPAD_AXIS_LEFT_X, "AxisLeftX"},
        {GLFW_GAMEPAD_AXIS_LEFT_Y, "AxisLeftY"},
        {GLFW_GAMEPAD_AXIS_RIGHT_X, "AxisRightX"},
        {GLFW_GAMEPAD_AXIS_RIGHT_Y, "AxisRightY"},
        {GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, "AxisLeftTrigger"},
        {GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, "AxisRightTrigger"}
    };

//    const std::map<uint32_t, std::string> m_inputGamepadAxis = {
//        {GLFW_GAMEPAD_AXIS_LEFT_X, "AxisLeftX"},
//        {GLFW_GAMEPAD_AXIS_LEFT_Y, "AxisLeftY"},
//        {GLFW_GAMEPAD_AXIS_RIGHT_X, "AxisLeftTrigger"},
//        {GLFW_GAMEPAD_AXIS_RIGHT_Y, "AxisRightX"},
//        {GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, "AxisRightY"},
//        {GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, "AxisRightTrigger"}
//    };
};

VectPairUI_t getPositionData(const INIReader &reader, const std::string & sectionName, const std::string &propertyName);
std::vector<uint32_t> convertStrToVectUI(const std::string &str);
std::optional<std::vector<uint32_t> > getBrutPositionData(const INIReader &reader, const std::string & sectionName, const std::string &propertyName);
std::vector<float> convertStrToVectFloat(const std::string &str);
std::vector<bool> convertStrToVectBool(const std::string &str);
std::vector<std::string> convertStrToVectStr(const std::string &str);
void fillPositionVerticalLine(const PairUI_t &origins, uint32_t size,
                              std::set<PairUI_t> &vectPos);
void fillPositionHorizontalLine(const PairUI_t &origins, uint32_t size,
                                std::set<PairUI_t> &vectPos);
void fillPositionRectangle(const PairUI_t &origins, const PairUI_t &size,
                           std::set<PairUI_t> &vectPos);
void fillPositionDiagLineUpLeft(const PairUI_t &origins, uint32_t size,
                                std::set<PairUI_t> &vectPos);
void fillPositionDiagLineDownLeft(const PairUI_t &origins, uint32_t size,
                                  std::set<PairUI_t> &vectPos);
void fillPositionDiagRectangle(const PairUI_t &origins, uint32_t size,
                               std::set<PairUI_t> &vectPos);
