#pragma once

#include <string>
#include <map>
#include <limits>
#include <cmath>
#include <vector>
#include <optional>

using PairUI_t = std::pair<uint32_t, uint32_t>;
using pairLong_t = std::pair<long, long>;
using PairFloat_t = std::pair<float, float>;
using PairDouble_t = std::pair<double, double>;
using PairPairFloatStr_t = std::pair<PairFloat_t, std::string>;
enum class AnimationMode_e
{
    STANDART,
    RETURN
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
    TOTAL_COMPONENTS
};

enum class MainMenuCursorPos_e
{
    RETURN_TO_GAME,
    SOUND_CONF,
    DISPLAY_CONF,
    INPUT_CONF,
    NEW_GAME,
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

enum class DisplayMenuCursorPos_e
{
    RESOLUTION_SETTING,
    QUALITY_SETTING,
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
    TELEPORT_ANIM_CT,
    DEAD_CORPSE_CT
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
    BASE,
    TRANSITION_LEVEL,
    DISPLAY,
    SOUND,
    INPUT,
    NEW_KEY
};

inline const float MAX_SOUND_DISTANCE = 1000.0f;
inline const float EJECT_TIME = 0.5F;
inline const float RAYCAST_VERTICAL_SIZE = 1.0f;
inline const float STD_FONT_SIZE = 0.06f;
inline const float MENU_FONT_SIZE = 0.10f;
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
inline const float ENEMY_RAY = LEVEL_HALF_TILE_SIZE_PX;
inline const float PLAYER_RAY = LEVEL_HALF_TILE_SIZE_PX - 2.0f;
inline const float MAP_LOCAL_CENTER_X_GL = 0.75f;
inline const float MAP_LOCAL_CENTER_Y_GL = -0.75f;
inline const float MAP_LOCAL_SIZE_GL = 0.50f;
inline const float EMPTY_VALUE = -10000.0f;
inline const float EPSILON_FLOAT = std::numeric_limits<float>::epsilon();
inline const float TAN_PI_QUARTER = std::tan(45.0f);
inline const float CONE_VISION = 60.0f;
inline const float MAX_SOUND_MENU_SIZE = 0.7f;
inline const float HALF_CONE_VISION = CONE_VISION / 2.0f;
inline const float DOUBLE_CONE_VISION = CONE_VISION * 2.0f;
inline const uint32_t RAYCAST_LINE_NUMBER = 200, RAYCAST_DEPTH = 20, MAX_SHOTS = 5,
RAYCAST_GROUND_CEILING_NUMBER = RAYCAST_LINE_NUMBER / 2.0f;
inline const float SCREEN_VERT_BACKGROUND_GL_STEP = 1.0f / RAYCAST_GROUND_CEILING_NUMBER;
inline const float SCREEN_HORIZ_BACKGROUND_GL_STEP = 2.0f / RAYCAST_LINE_NUMBER;
inline const PairFloat_t CURSOR_GL_SIZE = {0.20f, 0.25f};

inline const std::string STR_PLAYER_LIFE = "LIFE::";
inline const std::string STR_PLAYER_AMMO = "AMMO::";
inline const std::string SHADER_DIR_STR = "../Nietsneflow3d/Ressources/Shader/";
inline const std::string LEVEL_RESSOURCES_DIR_STR = "../Nietsneflow3d/Ressources/";
inline const std::string TEXTURES_DIR_STR = "../Nietsneflow3d/Ressources/Textures/";
inline const std::map<MenuMode_e, PairPairFloatStr_t> MAP_MENU_DATA =
{
    {MenuMode_e::BASE,
     PairPairFloatStr_t{{-0.5f, 0.5f},
      "RETURN TO GAME\\SOUND CONF\\DISPLAY CONF\\INPUT CONF\\NEW GAME\\EXIT GAME"}},
    {MenuMode_e::TRANSITION_LEVEL,
     PairPairFloatStr_t{{-0.5f, 0.5f}, "CONTINUE"}},
    {MenuMode_e::SOUND,
     PairPairFloatStr_t{{-0.7f, 0.5f}, "MUSIC VOLUME\\EFFECTS VOLUME\\RETURN"}},
    {MenuMode_e::DISPLAY,
     PairPairFloatStr_t{{-0.7f, 0.5f}, "SET RESOLUTION\\SET PICTURE QUALITY\\FULLSCREEN\\RETURN\\VALID"}},
    {MenuMode_e::INPUT,
     PairPairFloatStr_t{{-0.7f, 0.8f}, "MOVE FORWARD\\MOVE BACKWARD\\STRAFE LEFT\\"
         "STRAFE RIGHT\\TURN LEFT\\TURN RIGHT\\ACTION\\SHOOT\\PREVIOUS WEAPON\\NEXT WEAPON\\DEFAULT\\RETURN\\VALID"}
    },
    {MenuMode_e::NEW_KEY,
     PairPairFloatStr_t{{-0.6f, 0.1f}, "ENTER NEW KEY FOR ACTION :\\"}
    }
};
inline const std::map<Shader_e, std::string> SHADER_ID_MAP
{
    {Shader_e::COLOR_S, "ceilingFloor"},
    {Shader_e::TEXTURE_S, "wall"}
};
