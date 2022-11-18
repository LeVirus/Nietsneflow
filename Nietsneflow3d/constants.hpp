#pragma once

#include <string>
#include <map>
#include <limits>
#include <cmath>
#include <vector>
#include <optional>
#include <OpenGLUtils/glheaders.hpp>

using PairUI_t = std::pair<uint32_t, uint32_t>;
using pairLong_t = std::pair<long, long>;
using PairFloat_t = std::pair<float, float>;
using PairDouble_t = std::pair<double, double>;
using PairPairFloatStr_t = std::pair<PairFloat_t, std::string>;

enum class AnimationMode_e
{
    STANDART,
    RETURN,
    CONSTANT
};

enum class ControlKey_e
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    STRAFE_LEFT,
    STRAFE_RIGHT,
    TURN_LEFT,
    TURN_RIGHT,
    ACTION,
    SHOOT,
    PREVIOUS_WEAPON,
    NEXT_WEAPON,
    TOTAL
};

enum class AttackType_e
{
    MELEE,
    BULLETS,
    VISIBLE_SHOTS
};

enum Components_e
{
    POSITION_VERTEX_COMPONENT,
    SPRITE_TEXTURE_COMPONENT,
    MEM_SPRITE_DATA_COMPONENT,
    COLOR_VERTEX_COMPONENT,
    MAP_COORD_COMPONENT,
    INPUT_COMPONENT,
    CIRCLE_COLLISION_COMPONENT,
    SEGMENT_COLLISION_COMPONENT,
    RECTANGLE_COLLISION_COMPONENT,
    FPS_VISIBLE_STATIC_ELEMENT_COMPONENT,//no raycast element
    GENERAL_COLLISION_COMPONENT,
    MOVEABLE_COMPONENT,
    VISION_COMPONENT,
    TIMER_COMPONENT,
    DOOR_COMPONENT,
    PLAYER_CONF_COMPONENT,
    ENEMY_CONF_COMPONENT,
    MEM_POSITIONS_VERTEX_COMPONENT,
    WRITE_COMPONENT,
    SHOT_CONF_COMPONENT,
    OBJECT_CONF_COMPONENT,
    IMPACT_CONF_COMPONENT,
    WEAPON_COMPONENT,
    MOVEABLE_WALL_CONF_COMPONENT,
    TRIGGER_COMPONENT,
    TELEPORT_COMPONENT,
    MEM_FPS_GLSIZE_COMPONENT,
    BARREL_COMPONENT,
    AUDIO_COMPONENT,
    CHECKPOINT_COMPONENT,
    WALL_MULTI_SPRITE_CONF,
    LOG_COMPONENT,
    TOTAL_COMPONENTS
};

enum class ConfirmCursorPos_e
{
    TRUE,
    FALSE,
    TOTAL
};

enum class TitleMenuCursorPos_e
{
    NEW_GAME,
    LOAD_GAME,
    DISPLAY_CONF,
    SOUND_CONF,
    INPUT_CONF,
    PLAY_CUSTOM_LEVELS,
    QUIT_GAME,
    TOTAL
};

enum class MainMenuCursorPos_e
{
    RETURN_TO_GAME,
    SOUND_CONF,
    DISPLAY_CONF,
    INPUT_CONF,
    NEW_GAME,
    LOAD_GAME,
    RESTART_FROM_LAST_CHECKPOINT,
    RESTART_LEVEL,
    PLAY_CUSTOM_LEVELS,
    QUIT_GAME,
    TOTAL
};

enum class SoundMenuCursorPos_e
{
    MUSIC_VOLUME,
    EFFECTS_VOLUME,
    RETURN,
    TOTAL
};

enum class NewGameMenuCursorPos_e
{
    FIRST,
    SECOND,
    THIRD,
    RETURN,
    TOTAL
};

enum class LoadGameMenuCursorPos_e
{
    FIRST,
    SECOND,
    THIRD,
    RETURN,
    TOTAL
};

enum class LoadCustomGameMenuCursorPos_e
{
    FIRST,
    SECOND,
    THIRD,
    FOURTH,
    FIFTH,
    PREVIOUS,
    NEXT,
    RETURN,
    TOTAL
};

enum class DisplayMenuCursorPos_e
{
    RESOLUTION_SETTING,
    FULLSCREEN,
    RETURN,
    VALID,
    TOTAL
};

enum class InputMenuCursorPos_e
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    STRAFE_LEFT,
    STRAFE_RIGHT,
    TURN_LEFT,
    TURN_RIGHT,
    ACTION,
    SHOOT,
    PREVIOUS_WEAPON,
    NEXT_WEAPON,
    TURN_SENSITIVITY,
    DEFAULT,
    RETURN,
    VALID,
    TOTAL
};

enum class WallShapeINI_e
{
    RECTANGLE,
    VERT_LINE,
    HORIZ_LINE,
    POINT,
    DIAG_RECT,
    DIAG_UP_LEFT,
    DIAG_DOWN_LEFT,
    TOTAL
};

enum class Systems_e
{
    COLOR_DISPLAY_SYSTEM,
    MAP_DISPLAY_SYSTEM,
    INPUT_SYSTEM,
    COLLISION_SYSTEM,
    FIRST_PERSON_DISPLAY_SYSTEM,
    VISION_SYSTEM,
    DOOR_SYSTEM,
    STATIC_DISPLAY_SYSTEM,
    IA_SYSTEM,
    SOUND_SYSTEM
};

enum class DisplayType_e
{
    COLOR,
    SIMPLE_TEXTURE,
    TEXTURED_TILE,
    TOTAL
};

enum class Direction_e
{
    NORTH,
    WEST,
    SOUTH,
    EAST
};

enum class TriggerBehaviourType_e
{
    ONCE,
    REVERSABLE,
    MULTI,
    AUTO
};

enum class TriggerWallMoveType_e
{
    WALL,
    BUTTON,
    GROUND
};

enum class MoveOrientation_e
{
    FORWARD,
    FORWARD_LEFT,
    FORWARD_RIGHT,
    BACKWARD,
    BACKWARD_LEFT,
    BACKWARD_RIGHT,
    LEFT,
    RIGHT
};

enum class EnemySpriteElementType_e
{
    STATIC_FRONT,
    STATIC_FRONT_LEFT,
    STATIC_FRONT_RIGHT,
    STATIC_BACK,
    STATIC_BACK_LEFT,
    STATIC_BACK_RIGHT,
    STATIC_LEFT,
    STATIC_RIGHT,
    ATTACK,
    DYING,
    TOUCHED
};

enum class EnemySpriteType_e
{
    STATIC_FRONT,
    STATIC_FRONT_LEFT,
    STATIC_FRONT_RIGHT,
    STATIC_BACK,
    STATIC_BACK_LEFT,
    STATIC_BACK_RIGHT,
    STATIC_LEFT,
    STATIC_RIGHT,
    ATTACK,
    DYING,
    TOUCHED,
    TOTAL_SPRITE
};

enum class WallSpriteType_e
{
    STATIC_A,
    STATIC_B,
    TOTAL_SPRITE
};

enum class LevelStaticElementType_e
{
    GROUND,
    CEILING,
    OBJECT,
    IMPACT,
    TELEPORT
};

enum class Shader_e
{
    COLOR_S,
    TEXTURE_S,
    TOTAL_SHADER_S
};

enum class BaseShapeTypeGL_e
{
    TRIANGLE,
    RECTANGLE,
    DOUBLE_RECT,
    NONE
};

enum class DisplayMode_e
{
    WALL_DM,
    DOOR_DM,
    STANDART_DM
};


enum class CollisionTag_e
{
    PLAYER_CT,
    PLAYER_ACTION_CT,
    WALL_CT,
    DOOR_CT,
    ENEMY_CT,
    BULLET_PLAYER_CT,
    EXPLOSION_CT,
    HIT_PLAYER_CT,
    BULLET_ENEMY_CT,
    GHOST_CT,
    OBJECT_CT,
    EXIT_CT,
    STATIC_SET_CT,
    TRIGGER_CT,
    TELEPORT_CT,
    IMPACT_CT,
    BARREL_CT,
    CHECKPOINT_CT,
    SECRET_CT,
    TELEPORT_ANIM_CT,
    DEAD_CORPSE_CT,
    LOG_CT,
    DETECT_MAP_CT
};

enum class CollisionShape_e
{
    CIRCLE_C,
    RECTANGLE_C,
    SEGMENT_C
};

enum class LevelCaseType_e
{
    WALL_LC,
    DOOR_LC,
    EMPTY_LC,
    WALL_MOVE_LC
};


enum class MenuMode_e
{
    TITLE,
    BASE,
    TRANSITION_LEVEL,
    DISPLAY,
    SOUND,
    INPUT,
    NEW_GAME,
    LOAD_GAME,
    CONFIRM_QUIT_INPUT_FORM,
    CONFIRM_LOADING_GAME_FORM,
    CONFIRM_RESTART_LEVEL,
    CONFIRM_RESTART_FROM_LAST_CHECKPOINT,
    CONFIRM_QUIT_GAME,
    NEW_KEY,
    LOAD_CUSTOM_LEVEL,
    LEVEL_PROLOGUE,
    LEVEL_EPILOGUE
};

const uint32_t MIN_TURN_SENSITIVITY = 20, MAX_TURN_SENSITIVITY = 100,
DIFF_TOTAL_SENSITIVITY = MAX_TURN_SENSITIVITY - MIN_TURN_SENSITIVITY;
inline const uint32_t ENCRYPT_KEY_STANDARD_LEVEL = 17;
inline const uint32_t ENCRYPT_KEY_CUSTOM_LEVEL = 52;
inline const uint32_t ENCRYPT_KEY_CONF_FILE = 42;
inline const uint32_t CUSTOM_MENU_SECTION_SIZE = 9;
inline const float FPS_VALUE = 1.0f / 60.0f;
inline const float MAX_SOUND_DISTANCE = 1000.0f;
inline const float DOWN_WEAPON_POS_Y = -1.02f;
inline const uint32_t EJECT_CYCLE_TIME = 0.5f / FPS_VALUE;
inline const float RAYCAST_VERTICAL_SIZE = 1.0f;
inline const float STD_FONT_SIZE = 0.06f;
inline const float MENU_FONT_SIZE = 0.075f;
inline const float PI = 3.14159265359f;
inline const float PI_DOUBLE = PI * 2.0f;
inline const float PI_QUARTER = PI / 4.0f;
inline const float PI_HALF = PI / 2.0f;
inline const float PLAYER_RAY_DISPLAY = 0.03f;
inline const float LEVEL_TILE_SIZE_PX = 30.0f;
inline const float WIDTH_DOOR_SIZE_PX = LEVEL_TILE_SIZE_PX / 9.0f;
inline const float DOOR_CASE_POS_PX = WIDTH_DOOR_SIZE_PX * 4.0f;
inline const float LEVEL_HALF_TILE_SIZE_PX = LEVEL_TILE_SIZE_PX / 2.0f;
inline const float LEVEL_THIRD_TILE_SIZE_PX = LEVEL_TILE_SIZE_PX / 3.0f;
inline const float LEVEL_TWO_THIRD_TILE_SIZE_PX = (LEVEL_TILE_SIZE_PX / 3.0f) * 2.0f;
inline const float DETECT_RECT_SHAPE_SIZE = LEVEL_TILE_SIZE_PX * 5.0f;
inline const float DETECT_RECT_SHAPE_HALF_SIZE = DETECT_RECT_SHAPE_SIZE / 2.0f;
inline const float ENEMY_RAY = LEVEL_HALF_TILE_SIZE_PX;
inline const float PLAYER_RAY = LEVEL_HALF_TILE_SIZE_PX - 2.0f;
inline const float MAP_LOCAL_CENTER_X_GL = 0.5f;
inline const float MAP_LOCAL_CENTER_Y_GL = -0.5f;
inline const float MAP_FULL_TOP_LEFT_X_GL = -0.85f;
inline const float MAP_FULL_TOP_LEFT_Y_GL = 0.85f;
inline const float FULL_MAP_SIZE_GL = MAP_FULL_TOP_LEFT_Y_GL * 2.0f;
inline const float MAP_LOCAL_SIZE_GL = 1.0f;
inline const float EMPTY_VALUE = -10000.0f;
inline const float EPSILON_FLOAT = std::numeric_limits<float>::epsilon();
inline const float EPSILON_DOUBLE = std::numeric_limits<double>::epsilon();
inline const float TAN_PI_QUARTER = std::tan(45.0f);
inline const float CONE_VISION = 60.0f;
inline const float LEFT_POS_STD_MENU_BAR = 0.10f;
inline const float MAX_BAR_MENU_SIZE = 0.5f;
inline const float HALF_CONE_VISION = CONE_VISION / 2.0f;
inline const float DOUBLE_CONE_VISION = CONE_VISION * 2.0f;
inline const float WEAPONS_PREVIEW_GL_POS_LEFT = -0.95f;
inline const uint32_t RAYCAST_LINE_NUMBER = 300, RAYCAST_DEPTH = 20, MAX_SHOTS = 5,
RAYCAST_GROUND_CEILING_NUMBER = RAYCAST_LINE_NUMBER / 2.5;
inline const float SCREEN_VERT_BACKGROUND_GL_STEP = 1.0f / RAYCAST_GROUND_CEILING_NUMBER;
inline const float SCREEN_HORIZ_BACKGROUND_GL_STEP = 2.0f / RAYCAST_LINE_NUMBER;
inline const double WRITE_LETTER_GL_OFFSET = 0.01;
inline const std::string SHADER_DIR_STR = "../Nietsneflow3d/Ressources/Shader/";
inline const std::string LEVEL_RESSOURCES_DIR_STR = "../Nietsneflow3d/Ressources/";
inline const std::string TEXTURES_DIR_STR = "../Nietsneflow3d/Ressources/Textures/";
inline const std::map<MenuMode_e, PairPairFloatStr_t> MAP_MENU_DATA =
{
    {MenuMode_e::TITLE,
     PairPairFloatStr_t{{-0.3f, 0.5f},
        "New Game\\Load Game\\Display\\Sound\\Input\\Play Custom Levels\\Exit Game"}},
    {MenuMode_e::BASE,
     PairPairFloatStr_t{{-0.4f, 0.5f},
      "Return To Game\\Sound\\Display\\Input\\New Game\\Load Game\\Restart From Last Checkpoint\\Restart Level\\Play Custom Levels\\Exit Game"}},
    {MenuMode_e::TRANSITION_LEVEL,
     PairPairFloatStr_t{{-0.5f, 0.5f}, ""}},
    {MenuMode_e::LEVEL_PROLOGUE,
     PairPairFloatStr_t{{-0.5f, 0.5f}, ""}},
    {MenuMode_e::LEVEL_EPILOGUE,
     PairPairFloatStr_t{{-0.5f, 0.5f}, ""}},
    {MenuMode_e::SOUND,
     PairPairFloatStr_t{{-0.7f, 0.5f}, "Music Volume\\Effects Volume\\Return"}},
    {MenuMode_e::DISPLAY,
     PairPairFloatStr_t{{-0.7f, 0.5f}, "Set Resolution\\Fullscreen\\Return\\Valid"}},
    {MenuMode_e::INPUT,
     PairPairFloatStr_t{{-0.7f, 0.7f}, "Move Forward\\Move Backward\\Strafe Left\\"
         "Strafe Right\\Turn Left\\Turn Right\\Action\\Shoot\\Previous Weapon\\Next Weapon\\Turn Sensitivity\\Default\\Return\\Valid"}
    },
    {MenuMode_e::NEW_GAME,
     PairPairFloatStr_t{{-0.6f, 0.5f}, "1\\2\\3\\Return"}
    },
    {MenuMode_e::LOAD_GAME,
     PairPairFloatStr_t{{-0.6f, 0.5f}, "1\\2\\3\\Return"}
    },
    {MenuMode_e::CONFIRM_QUIT_INPUT_FORM,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "Yes\\No"}
    },
    {MenuMode_e::CONFIRM_LOADING_GAME_FORM,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "Yes\\No"}
    },
    {MenuMode_e::CONFIRM_RESTART_LEVEL,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "Yes\\No"}
    },
    {MenuMode_e::CONFIRM_RESTART_FROM_LAST_CHECKPOINT,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "Yes\\No"}
    },
    {MenuMode_e::CONFIRM_QUIT_GAME,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "Yes\\No"}
    },
    {MenuMode_e::NEW_KEY,
     PairPairFloatStr_t{{-0.6f, 0.1f}, "Press Escape For Cancel\\Enter New Key For Action :\\"}
    },
    {MenuMode_e::LOAD_CUSTOM_LEVEL,
     PairPairFloatStr_t{{-0.7f, 0.7f}, "1\\2\\3\\4\\5\\Previous\\Next\\Return"}
    }
};
inline const std::map<Shader_e, std::string> SHADER_ID_MAP
{
    {Shader_e::COLOR_S, "ceilingFloor"},
    {Shader_e::TEXTURE_S, "wall"}
};

inline const std::map<uint32_t, std::string> INPUT_KEYBOARD_KEY_STRING = {
    {GLFW_KEY_SPACE, "Space"},
    {GLFW_KEY_APOSTROPHE, "Apostrophe"},
    {GLFW_KEY_COMMA, "Comma"},
    {GLFW_KEY_MINUS, "Minus"},
    {GLFW_KEY_PERIOD, "Period"},
    {GLFW_KEY_SLASH, "Slash"},
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
    {GLFW_KEY_SEMICOLON, "Semicolon"},
    {GLFW_KEY_EQUAL, "Equal"},
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
    {GLFW_KEY_LEFT_BRACKET, "Left Bracket"},
    {GLFW_KEY_BACKSLASH, "Backslash"},
    {GLFW_KEY_RIGHT_BRACKET, "Right Bracket"},
    {GLFW_KEY_GRAVE_ACCENT, "Grave Accent"},
    {GLFW_KEY_INSERT, "Insert"},
    {GLFW_KEY_RIGHT, "Right"},
    {GLFW_KEY_LEFT, "Left"},
    {GLFW_KEY_DOWN, "Down"},
    {GLFW_KEY_UP, "Up"},
    {GLFW_KEY_PAGE_UP, "Page Up"},
    {GLFW_KEY_PAGE_DOWN, "Page Down"},
    {GLFW_KEY_HOME, "Home"},
    {GLFW_KEY_END, "End"},
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
    {GLFW_KEY_KP_DECIMAL, "KP Decimal"},
    {GLFW_KEY_KP_DIVIDE, "KP Divide"},
    {GLFW_KEY_KP_MULTIPLY, "KP Multiply"},
    {GLFW_KEY_KP_SUBTRACT, "KP Substract"},
    {GLFW_KEY_KP_ADD, "KP Add"},
    {GLFW_KEY_KP_EQUAL, "KP Equal"},
    {GLFW_KEY_LEFT_SHIFT, "Left Shift"},
    {GLFW_KEY_LEFT_CONTROL, "Left Control"},
    {GLFW_KEY_LEFT_ALT, "Left Alt"},
    {GLFW_KEY_LEFT_SUPER, "Left Super"},
    {GLFW_KEY_RIGHT_SHIFT, "Right Shift"},
    {GLFW_KEY_RIGHT_CONTROL, "Right Control"},
    {GLFW_KEY_RIGHT_ALT, "Right Alt"},
    {GLFW_KEY_RIGHT_SUPER, "Right Super"},
    {GLFW_KEY_MENU, "Menu"}
};

inline const std::map<uint32_t, std::string> INPUT_MOUSE_KEY_STRING = {
    {GLFW_MOUSE_BUTTON_LEFT, "Mouse Left"},
    {GLFW_MOUSE_BUTTON_RIGHT, "Mouse Right"},
    {GLFW_MOUSE_BUTTON_MIDDLE, "Mouse Middle"},
    {GLFW_MOUSE_BUTTON_4, "Mouse 4"},
    {GLFW_MOUSE_BUTTON_5, "Mouse 5"},
    {GLFW_MOUSE_BUTTON_6, "Mouse 6"},
    {GLFW_MOUSE_BUTTON_7, "Mouse 7"},
    {GLFW_MOUSE_BUTTON_8, "Mouse 8"}
};

inline const std::map<uint32_t, std::string> INPUT_GAMEPAD_SIMPLE_BUTTONS_STRING = {
    {GLFW_GAMEPAD_BUTTON_A, "Button A"},
    {GLFW_GAMEPAD_BUTTON_B, "Button B"},
    {GLFW_GAMEPAD_BUTTON_X, "Button X"},
    {GLFW_GAMEPAD_BUTTON_Y, "Button Y"},
    {GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "Left Bumper"},
    {GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "Right Bumper"},
//    {GLFW_GAMEPAD_BUTTON_BACK, "Back"},
    {GLFW_GAMEPAD_BUTTON_START, "Start"},
    {GLFW_GAMEPAD_BUTTON_GUIDE, "Guide"},
    {GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "Left Thumb"},
    {GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "Right Thumb"},
    {GLFW_GAMEPAD_BUTTON_DPAD_UP, "Dpad Up"},
    {GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "Dpad Right"},
    {GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "Dpad Down"},
    {GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "Dpad Left"}
};

inline const std::map<uint32_t, std::string> INPUT_GAMEPAD_AXIS_STRING = {
    {GLFW_GAMEPAD_AXIS_LEFT_X, "Axis Left X"},
    {GLFW_GAMEPAD_AXIS_LEFT_Y, "Axis Left Y"},
    {GLFW_GAMEPAD_AXIS_RIGHT_X, "Axis Right X"},
    {GLFW_GAMEPAD_AXIS_RIGHT_Y, "Axis Right Y"},
    {GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, "Axis Left Trigger"},
    {GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, "Axis Right Trigger"}
};
