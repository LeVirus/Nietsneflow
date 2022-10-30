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
struct MemCheckpointElementsState;
struct MemCheckpointEnemiesState;
struct MemLevelLoadedData;

struct MemSpriteData
{
    uint16_t m_numSprite;
    PairFloat_t m_GLSize;
};

using SetStr_t = std::set<std::string>;
using MapVisibleShotData_t = std::map<std::string, std::pair<std::string ,std::vector<MemSpriteData>>>;
using PairImpactData_t = std::pair<std::vector<MemSpriteData>, MemSpriteData>;
using MapImpactData_t = std::map<std::string, PairImpactData_t>;
using pairI_t = std::pair<int, int>;
using PairStrPairFloat_t = std::pair<std::string, PairFloat_t>;

enum class LevelLoadState_e
{
    OK,
    END,
    FAIL
};

struct WeaponINIData
{
    std::vector<MemSpriteData> m_spritesData;
    uint32_t m_maxAmmo, m_simultaneousShots, m_lastAnimNum, m_order, m_damage;
    AttackType_e m_attackType;
    AnimationMode_e m_animMode;
    std::string m_visibleShootID, m_impactID, m_shotSound, m_reloadSound, m_weaponName;
    float m_animationLatency, m_shotVelocity;
    bool m_startingPossess;
    std::optional<float> m_damageCircleRay;
    std::optional<uint32_t> m_startingAmmoCount;
};

struct WallData
{
    std::vector<uint16_t> m_sprites;
    std::set<PairUI_t> m_TileGamePosition, m_removeGamePosition;
    std::vector<uint32_t> m_cyclesTime;
};

struct AssociatedTriggerData
{
    PairUI_t m_pos;
    MemSpriteData m_displayData;
};

struct MoveableWallData
{
    std::vector<uint16_t> m_sprites;
    std::vector<uint32_t> m_cyclesTime;
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
    std::optional<uint32_t> m_musicVolume, m_effectsVolume;
    //DISPLAY
    std::optional<bool> m_fullscreen;
    std::optional<int> m_resolutionWidth, m_resolutionHeight;
    //INPUT
    std::optional<std::array<MouseKeyboardInputState, static_cast<uint32_t>(ControlKey_e::TOTAL)>> m_arrayKeyboard;
    std::optional<std::array<GamepadInputState, static_cast<uint32_t>(ControlKey_e::TOTAL)>> m_arrayGamepad;
    std::optional<uint32_t> m_turnSensitivity;
};

struct DataLevelWriteMenu
{
    std::string m_date;
    uint32_t m_levelNum, m_checkpointNum;
};

struct LogStdData
{
    uint16_t m_spriteNum;
    PairFloat_t m_fpsSize;
};

struct LogLevelData
{
    std::string m_id, m_message;
    PairUI_t m_pos;
};

class LevelManager
{
public:
    LevelManager();
    void loadTextureData(const std::string &INIFileName);
    void loadStandardData(const std::string &INIFileName);
    void loadFontData(const std::string &INIFileName, Font_e type);
    bool loadSettingsData();
    LevelLoadState_e loadLevel(uint32_t levelNum, bool customLevel = false);
    void clearExistingPositionsElement();
    void saveAudioSettings(uint32_t musicVolume, uint32_t effectVolume);
    void saveDisplaySettings(const pairI_t &resolution, bool fullscreen);
    void saveInputSettings(const std::map<ControlKey_e, GamepadInputState> &gamepadArray,
                           const std::map<ControlKey_e, MouseKeyboardInputState> &keyboardArray);
    void saveTurnSensitivitySettings(uint32_t sensitivity);
    std::optional<MemLevelLoadedData> loadSavedGame(uint32_t saveNum);
    std::optional<MemPlayerConf> loadPlayerConf(bool beginLevel);
    std::unique_ptr<MemCheckpointElementsState> loadCheckpointDataSavedGame();
    std::map<uint32_t, std::pair<std::vector<uint32_t>, bool> > loadTriggerWallMoveableWallDataGameProgress();
    std::map<uint32_t, std::pair<uint32_t, bool> > loadMoveableWallDataGameProgress();
    std::array<std::optional<DataLevelWriteMenu>, 3> getExistingLevelNumSaves();
    void loadExistingCustomLevelFilename();
    //if encryptKey is nullopt no encryption
    bool loadIniFile(std::string_view path, std::optional<uint32_t> encryptKey);
    std::string saveGameProgress(const MemPlayerConf &playerConfBeginLevel, const MemPlayerConf &playerConfCheckpoint, uint32_t levelNum,
                          uint32_t numSaveFile, const MemCheckpointElementsState *checkpointData);
    inline std::vector<std::string> getExistingCustomLevel()
    {
        return m_existingCustomLevelsFilename;
    }
    inline const PictureData &getPictureData()const {return m_pictureData;}
    inline const Level &getLevel()const {return m_level;}
    inline const FontData &getFontData()const {return m_fontData;}
    inline std::string getGenericMenuSpriteName()const
    {
        return m_spriteGenericMenuName;
    }
    inline std::string getTitleMenuSpriteName()const
    {
        return m_spriteTitleMenuName;
    }
    inline std::string getLevelPrologue()const
    {
        return m_prologue;
    }
    inline std::string getLevelEpilogue()const
    {
        return m_epilogue;
    }
    inline std::string getLevelEpilogueMusic()const
    {
        return m_epilogueMusic;
    }
    inline std::string getPannelSpriteName()const
    {
        return m_spritePannelName;
    }
    inline std::string getLifeIconSpriteName()const
    {
        return m_spriteLifeName;
    }
    inline std::string getAmmoIconSpriteName()const
    {
        return m_spriteAmmoName;
    }

    inline std::string getFistIconSpriteName()const
    {
        return m_spriteFistName;
    }
    inline std::string getGunIconSpriteName()const
    {
        return m_spriteGunName;
    }
    inline std::string getShotgunIconSpriteName()const
    {
        return m_spriteShotgunName;
    }
    inline std::string getPlasmaRifleIconSpriteName()const
    {
        return m_spritePlasmaRifleName;
    }
    inline std::string getMachineGunIconSpriteName()const
    {
        return m_spriteMachineGunName;
    }
    inline std::string getBazookaIconSpriteName()const
    {
        return m_spriteBazookaName;
    }

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
    inline const std::vector<std::pair<PairUI_t, Direction_e>> &getCheckpointsData()const
    {
        return m_checkpointsPos;
    }
    inline const std::vector<PairUI_t> &getSecretsData()const
    {
        return m_secretsPos;
    }
    inline const std::vector<LogLevelData> &getLogsData()const
    {
        return m_logsLevelData;
    }
    inline const std::map<std::string, LogStdData> &getStdLogData()const
    {
        return m_logStdData;
    }
    inline const std::vector<PairStrPairFloat_t> &getWeaponsPreviewData()const
    {
        return m_weaponsPreviewData;
    }
    bool checkSavedGameExists(uint32_t saveNum) const;
private:
    std::string getGamepadKeyIniString(const GamepadInputState &gamepadInputState) const;
    void fillSettingsFileFromMemory();
    //texture and sprite loading
    void loadTexturePath();
    void loadSpriteData(const std::string &sectionName = "Sprite",
                        bool font = false, std::optional<Font_e> type = {});
    bool loadBackgroundData();
    void loadMusicData();
    //Level element datas loading
    bool loadLevelData();
    bool loadPositionPlayerData(uint32_t levelNum);
    void loadGeneralStaticElements(LevelStaticElementType_e elementType);
    void loadPositionStaticElements();
    void loadBarrelElements();
    std::string saveLevelGameProgress(const MemPlayerConf &playerConfBeginLevel, const MemPlayerConf &playerConfCheckpoint, uint32_t levelNum, bool beginLevel);
    void savePlayerGear(bool beginLevel, const MemPlayerConf &playerConf);
    void saveElementsGameProgress(const MemCheckpointElementsState &checkpointData);
    void saveMoveableWallDataGameProgress(const std::map<uint32_t, std::pair<uint32_t, bool> > &moveableWallData);
    void saveTriggerWallMoveableWallDataGameProgress(const std::map<uint32_t, std::pair<std::vector<uint32_t>, bool> > &triggerWallMoveableWallData);
    void saveEnemiesDataGameProgress(const std::vector<MemCheckpointEnemiesState> &enemiesData);
    std::vector<MemCheckpointEnemiesState> loadEnemiesDataGameProgress();
    void saveStaticElementsDataGameProgress(const std::set<PairUI_t> &staticElementData);
    void saveRevealedMapGameProgress(const std::vector<PairUI_t> &revealedMapData);
    std::set<PairUI_t> loadStaticElementsDataGameProgress();
    std::vector<PairUI_t> loadRevealedMapDataGameProgress();
    void generateSavedFile(uint32_t numSaveFile);
    void readStandardStaticElement(StaticLevelElementData &staticElement,
                                   const std::string &sectionName,
                                   LevelStaticElementType_e elementType);
    bool fillStandartPositionVect(const std::string &sectionName,
                                  VectPairUI_t &vectPos);
    bool fillTeleportPositions(const std::string &sectionName);
    std::map<std::string, StaticLevelElementData>::iterator removeStaticElement(const std::string_view sectionName,
                                                                                LevelStaticElementType_e elementType);
    std::optional<PairUI_t> getPosition(const std::string_view sectionName, const std::string_view propertyName);
    bool fillWallPositionVect(const std::string &sectionName, const std::string &propertyName,
                              std::set<PairUI_t> &setPos);
    uint16_t getSpriteId(const std::string &sectionName);
    void loadVisibleShotDisplayData();
    void loadShotImpactDisplayData();
    void loadWeaponsData();
    void loadGeneralSoundData();
    void loadBarrelsData();
    void loadLogData();
    void loadExit();
    void loadVisualTeleportData();
    void loadTriggerElements();
    void loadSpriteData(const std::string &sectionName, StaticLevelElementData &staticElement);
    void loadDisplayData(std::string_view sectionName, std::string_view subSectionName);
    void loadWeaponData(std::string_view sectionName, uint32_t numIt);
    void loadWallData();
    void loadPositionWall();
    void loadTriggerLevelData(const std::string &sectionName);
    void loadDoorData();
    void loadPositionDoorData();
    void loadEnemyData();
    //return true if one enemy is end level
    bool loadPositionEnemyData();
    void loadPositionCheckpointsData();
    void loadPositionSecretsData();
    void loadPositionLogsData();
    void loadPrologueAndEpilogue();
    void loadUtilsData();
    PairStrPairFloat_t loadPreviewWeaponData(const std::string &iniObject);
    void loadEnemySprites(const std::string &sectionName,
                          EnemySpriteElementType_e spriteTypeEnum, EnemyData &enemyData);
    void deleteWall(const PairUI_t &coord);
    bool loadPositionExit();
    std::vector<uint16_t> getVectSpriteNum(const std::string_view section, const std::string_view param);
    std::vector<PairFloat_t> getVectSpriteGLSize(const std::string_view section, const std::string_view weightParam,
                                                  const std::string_view heightParam);
    std::optional<std::vector<uint32_t>> getBrutPositionData(const std::string & sectionName,
                                                              const std::string &propertyName);
    VectPairUI_t getPositionData(const std::string & sectionName, const std::string &propertyName);
private:
    std::vector<std::string> m_existingCustomLevelsFilename;
    inipp::Ini<char> m_ini;
    std::ofstream m_outputStream;
    std::ifstream m_inputStream;
    PictureData m_pictureData;
    FontData m_fontData;
    Level m_level;
    std::string m_spritePannelName, m_spriteLifeName, m_spriteAmmoName,
    m_spriteFistName, m_spriteGunName, m_spriteShotgunName,
    m_spritePlasmaRifleName, m_spriteMachineGunName, m_spriteBazookaName, m_spriteGenericMenuName, m_spriteTitleMenuName;
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
    std::vector<PairUI_t> m_secretsPos;
    std::vector<LogLevelData> m_logsLevelData;
    std::string m_prologue, m_epilogue, m_epilogueMusic;
    //first ID, second Std data
    std::map<std::string, LogStdData> m_logStdData;
    std::vector<std::pair<PairUI_t, Direction_e>> m_checkpointsPos;
    std::vector<PairStrPairFloat_t> m_weaponsPreviewData;
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
    const std::map<std::string, uint32_t> m_inputMouseKeyString = {
        {"Mouse Left", GLFW_MOUSE_BUTTON_LEFT},
        {"Mouse Right", GLFW_MOUSE_BUTTON_RIGHT},
        {"Mouse Middle", GLFW_MOUSE_BUTTON_MIDDLE},
        {"Mouse 4", GLFW_MOUSE_BUTTON_4},
        {"Mouse 5", GLFW_MOUSE_BUTTON_5},
        {"Mouse 6", GLFW_MOUSE_BUTTON_6},
        {"Mouse 7", GLFW_MOUSE_BUTTON_7},
        {"Mouse 8", GLFW_MOUSE_BUTTON_8}
    };
    const std::map<std::string, uint32_t> m_inputKeyboardKeyString = {
        {"Space", GLFW_KEY_SPACE},
        {"Apostrophe", GLFW_KEY_APOSTROPHE},
        {"Comma", GLFW_KEY_COMMA},
        {"Minus", GLFW_KEY_MINUS},
        {"Period", GLFW_KEY_PERIOD},
        {"Slash", GLFW_KEY_SLASH},
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
        {"Semicolon", GLFW_KEY_SEMICOLON},
        {"Equal", GLFW_KEY_EQUAL},
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
        {"Left Bracket", GLFW_KEY_LEFT_BRACKET},
        {"Backslash", GLFW_KEY_BACKSLASH},
        {"Right Bracket", GLFW_KEY_RIGHT_BRACKET},
        {"Grave Accent", GLFW_KEY_GRAVE_ACCENT},
        {"Insert", GLFW_KEY_INSERT},
        {"Right", GLFW_KEY_RIGHT},
        {"Left", GLFW_KEY_LEFT},
        {"Down", GLFW_KEY_DOWN},
        {"Up", GLFW_KEY_UP},
        {"Page Up", GLFW_KEY_PAGE_UP},
        {"Page Down", GLFW_KEY_PAGE_DOWN},
        {"Home", GLFW_KEY_HOME},
        {"End", GLFW_KEY_END},
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
        {"KP Decimal", GLFW_KEY_KP_DECIMAL},
        {"KP Divide", GLFW_KEY_KP_DIVIDE},
        {"KP Multiply", GLFW_KEY_KP_MULTIPLY},
        {"KP Substract", GLFW_KEY_KP_SUBTRACT},
        {"KP Add", GLFW_KEY_KP_ADD},
        {"KP Equal", GLFW_KEY_KP_EQUAL},
        {"Left Shift", GLFW_KEY_LEFT_SHIFT},
        {"Left Control", GLFW_KEY_LEFT_CONTROL},
        {"Left Alt", GLFW_KEY_LEFT_ALT},
        {"Left Super", GLFW_KEY_LEFT_SUPER},
        {"Right Shift", GLFW_KEY_RIGHT_SHIFT},
        {"Right Control", GLFW_KEY_RIGHT_CONTROL},
        {"Right Alt", GLFW_KEY_RIGHT_ALT},
        {"Right Super", GLFW_KEY_RIGHT_SUPER},
        {"Menu", GLFW_KEY_MENU}
    };
    const std::map<std::string, GamepadInputState> m_inputGamepadKeyString = {
        {"ButtonA", {true, GLFW_GAMEPAD_BUTTON_A, {}}},
        {"ButtonB", {true, GLFW_GAMEPAD_BUTTON_B, {}}},
        {"ButtonX", {true, GLFW_GAMEPAD_BUTTON_X, {}}},
        {"ButtonY", {true, GLFW_GAMEPAD_BUTTON_Y, {}}},
        {"ButtonLeftBumper", {true, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, {}}},
        {"ButtonRightBumper", {true, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, {}}},
//        {"ButtonBack", {true, GLFW_GAMEPAD_BUTTON_BACK, {}}},
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
//        {GLFW_GAMEPAD_BUTTON_BACK, "ButtonBack"},
        {GLFW_GAMEPAD_BUTTON_START, "ButtonStart"},
        {GLFW_GAMEPAD_BUTTON_GUIDE, "ButtonGuide"},
        {GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "ButtonLeftThumb"},
        {GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "ButtonRightThumb"},
        {GLFW_GAMEPAD_BUTTON_DPAD_UP, "ButtonUp"},
        {GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "ButtonRight"},
        {GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "ButtonDown"},
        {GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "ButtonLeft"}
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
std::string getStrDate();
std::optional<bool> toBool(const std::string &str);
std::string encrypt(const std::string &str, uint32_t key);
std::string decrypt(const std::string &str, uint32_t key);
std::vector<uint32_t> convertStrToVectUI(const std::string &str);
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
