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
inline const float MAX_BAR_MENU_SIZE = 0.7f;
inline const float HALF_CONE_VISION = CONE_VISION / 2.0f;
inline const float DOUBLE_CONE_VISION = CONE_VISION * 2.0f;
inline const float WEAPONS_PREVIEW_GL_POS_LEFT = -0.95f;
inline const uint32_t RAYCAST_LINE_NUMBER = 300, RAYCAST_DEPTH = 20, MAX_SHOTS = 5,
RAYCAST_GROUND_CEILING_NUMBER = RAYCAST_LINE_NUMBER / 2.5;
inline const float SCREEN_VERT_BACKGROUND_GL_STEP = 1.0f / RAYCAST_GROUND_CEILING_NUMBER;
inline const float SCREEN_HORIZ_BACKGROUND_GL_STEP = 2.0f / RAYCAST_LINE_NUMBER;
inline const PairFloat_t CURSOR_GL_SIZE = {0.20f, 0.25f};
inline const double WRITE_LETTER_GL_OFFSET = 0.01;
inline const std::string STR_PLAYER_LIFE;
inline const std::string STR_PLAYER_AMMO;
inline const std::string SHADER_DIR_STR = "../Nietsneflow3d/Ressources/Shader/";
inline const std::string LEVEL_RESSOURCES_DIR_STR = "../Nietsneflow3d/Ressources/";
inline const std::string TEXTURES_DIR_STR = "../Nietsneflow3d/Ressources/Textures/";
inline const std::map<MenuMode_e, PairPairFloatStr_t> MAP_MENU_DATA =
{
    {MenuMode_e::TITLE,
     PairPairFloatStr_t{{-0.5f, 0.5f},
        "NEW GAME\\LOAD GAME\\DISPLAY CONF\\SOUND CONF\\INPUT CONF\\PLAY CUSTOM LEVELS\\EXIT GAME"}},
    {MenuMode_e::BASE,
     PairPairFloatStr_t{{-0.5f, 0.5f},
      "RETURN TO GAME\\SOUND CONF\\DISPLAY CONF\\INPUT CONF\\NEW GAME\\LOAD GAME\\RESTART FROM LAST CHECKPOINT\\RESTART LEVEL\\PLAY CUSTOM LEVELS\\EXIT GAME"}},
    {MenuMode_e::TRANSITION_LEVEL,
     PairPairFloatStr_t{{-0.5f, 0.5f}, "CONTINUE"}},
    {MenuMode_e::LEVEL_PROLOGUE,
     PairPairFloatStr_t{{-0.5f, 0.5f}, "CONTINUE"}},
    {MenuMode_e::LEVEL_EPILOGUE,
     PairPairFloatStr_t{{-0.5f, 0.5f}, "CONTINUE"}},
    {MenuMode_e::SOUND,
     PairPairFloatStr_t{{-0.7f, 0.5f}, "MUSIC VOLUME\\EFFECTS VOLUME\\RETURN"}},
    {MenuMode_e::DISPLAY,
     PairPairFloatStr_t{{-0.7f, 0.5f}, "SET RESOLUTION\\FULLSCREEN\\RETURN\\VALID"}},
    {MenuMode_e::INPUT,
     PairPairFloatStr_t{{-0.7f, 0.7f}, "MOVE FORWARD\\MOVE BACKWARD\\STRAFE LEFT\\"
         "STRAFE RIGHT\\TURN LEFT\\TURN RIGHT\\ACTION\\SHOOT\\PREVIOUS WEAPON\\NEXT WEAPON\\TURN SENSITIVITY\\DEFAULT\\RETURN\\VALID"}
    },
    {MenuMode_e::NEW_GAME,
     PairPairFloatStr_t{{-0.7f, 0.7f}, "1\\2\\3\\RETURN"}
    },
    {MenuMode_e::LOAD_GAME,
     PairPairFloatStr_t{{-0.7f, 0.7f}, "1\\2\\3\\RETURN"}
    },
    {MenuMode_e::CONFIRM_QUIT_INPUT_FORM,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "YES\\NO"}
    },
    {MenuMode_e::CONFIRM_LOADING_GAME_FORM,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "YES\\NO"}
    },
    {MenuMode_e::CONFIRM_RESTART_LEVEL,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "YES\\NO"}
    },
    {MenuMode_e::CONFIRM_RESTART_FROM_LAST_CHECKPOINT,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "YES\\NO"}
    },
    {MenuMode_e::CONFIRM_QUIT_GAME,
     PairPairFloatStr_t{{-0.1f, 0.0f}, "YES\\NO"}
    },
    {MenuMode_e::NEW_KEY,
     PairPairFloatStr_t{{-0.6f, 0.1f}, "PRESS ESCAPE FOR CANCEL\\ENTER NEW KEY FOR ACTION :\\"}
    },
    {MenuMode_e::LOAD_CUSTOM_LEVEL,
     PairPairFloatStr_t{{-0.7f, 0.7f}, "1\\2\\3\\4\\5\\PREVIOUS\\NEXT\\RETURN"}
    }
};
inline const std::map<Shader_e, std::string> SHADER_ID_MAP
{
    {Shader_e::COLOR_S, "ceilingFloor"},
    {Shader_e::TEXTURE_S, "wall"}
};

inline const std::map<uint32_t, std::string> INPUT_KEYBOARD_KEY_STRING = {
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
};

inline const std::map<uint32_t, std::string> INPUT_MOUSE_KEY_STRING = {
    {GLFW_MOUSE_BUTTON_LEFT, "MOUSE LEFT"},
    {GLFW_MOUSE_BUTTON_RIGHT, "MOUSE RIGHT"},
    {GLFW_MOUSE_BUTTON_MIDDLE, "MOUSE MIDDLE"},
    {GLFW_MOUSE_BUTTON_4, "MOUSE 4"},
    {GLFW_MOUSE_BUTTON_5, "MOUSE 5"},
    {GLFW_MOUSE_BUTTON_6, "MOUSE 6"},
    {GLFW_MOUSE_BUTTON_7, "MOUSE 7"},
    {GLFW_MOUSE_BUTTON_8, "MOUSE 8"}
};

inline const std::map<uint32_t, std::string> INPUT_GAMEPAD_SIMPLE_BUTTONS_STRING = {
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
};

inline const std::map<uint32_t, std::string> INPUT_GAMEPAD_AXIS_STRING = {
    {GLFW_GAMEPAD_AXIS_LEFT_X, "AXIS LEFT X"},
    {GLFW_GAMEPAD_AXIS_LEFT_Y, "AXIS LEFT Y"},
    {GLFW_GAMEPAD_AXIS_RIGHT_X, "AXIS RIGHT X"},
    {GLFW_GAMEPAD_AXIS_RIGHT_Y, "AXIS RIGHT Y"},
    {GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, "AXIS LEFT TRIGGER"},
    {GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, "AXIS RIGHT TRIGGER"}
};
