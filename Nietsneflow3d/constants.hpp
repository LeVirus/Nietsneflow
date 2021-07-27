#pragma once

#include <string>
#include <map>
#include <limits>
#include <cmath>
#include <vector>
#include <optional>

inline const uint32_t SEGMENT_SHOT_NUMBER = 4;

using pairUI_t = std::pair<uint32_t, uint32_t>;
using pairLong_t = std::pair<long, long>;
using pairFloat_t = std::pair<float, float>;
using pairDouble_t = std::pair<double, double>;
using AmmoContainer_t = std::array<std::optional<uint32_t>, SEGMENT_SHOT_NUMBER>;

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
    TOTAL_COMPONENTS
};

enum class CurrentMenuCursorPos_e
{
    RETURN_TO_GAME,
    NEW_GAME,
    QUIT_GAME,
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
    IA_SYSTEM
};

enum class DisplayType_e
{
    COLOR,
    SIMPLE_TEXTURE,
    TEXTURED_TILE
};

enum class Direction_e
{
    NORTH,
    WEST,
    SOUTH,
    EAST
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
    TOUCHED,
    VISIBLE_SHOOT,
    VISIBLE_SHOOT_DESTRUCT
};

enum class EnemySpriteType_e
{
    STATIC_FRONT_A,
    STATIC_FRONT_B,
    STATIC_MID_FRONT_LEFT_A,
    STATIC_MID_FRONT_LEFT_B,
    STATIC_MID_FRONT_RIGHT_A,
    STATIC_MID_FRONT_RIGHT_B,
    STATIC_BACK_A,
    STATIC_BACK_B,
    STATIC_MID_BACK_LEFT_A,
    STATIC_MID_BACK_LEFT_B,
    STATIC_MID_BACK_RIGHT_A,
    STATIC_MID_BACK_RIGHT_B,
    STATIC_LEFT_A,
    STATIC_LEFT_B,
    STATIC_RIGHT_A,
    STATIC_RIGHT_B,//16
    ATTACK_A,
    ATTACK_B,
    ATTACK_C,
    DYING_A,
    DYING_B,
    DYING_C,
    DYING_D,
    DEAD,
    TOUCHED,
    TOTAL_SPRITE
};

enum class WeaponsSpriteType_e
{
    FIST_STATIC,
    FIST_ATTACK_A,
    FIST_ATTACK_B,
    FIST_ATTACK_C,
    GUN_STATIC,
    GUN_SHOOT,
    SHOTGUN_STATIC,
    SHOTGUN_SHOOT,
    SHOTGUN_RELOAD_A,
    SHOTGUN_RELOAD_B,
    SHOTGUN_RELOAD_C,
    TOTAL_SPRITE
};

enum class WeaponsType_e
{
    FIST,
    GUN,
    SHOTGUN,
    TOTAL
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
    IMPACT
};

enum class Shader_e
{
    COLOR_S,
    TEXTURE_S,
//    TEXTURED_WALL_S,
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
    HIT_PLAYER_CT,
    BULLET_ENEMY_CT,
    GHOST_CT,
    OBJECT_CT,
    EXIT_CT,
    STATIC_SET_CT,
    IMPACT_CT
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
    EMPTY_LC
};

enum class ShotPhase_e
{
    NORMAL,
    SHOT_DESTRUCT_A,
    SHOT_DESTRUCT_B,
    SHOT_DESTRUCT_C,
    TOTAL
};

inline const float RAYCAST_VERTICAL_SIZE = 1.0f;
inline const float STD_FONT_SIZE = 0.06f;
inline const float MENU_FONT_SIZE = 0.15f;
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
inline const float ENEMY_RAY = LEVEL_HALF_TILE_SIZE_PX;
inline const float PLAYER_RAY = LEVEL_HALF_TILE_SIZE_PX - 2.0f;
inline const float MAP_LOCAL_CENTER_X_GL = 0.75f;
inline const float MAP_LOCAL_CENTER_Y_GL = -0.75f;
inline const float MAP_LOCAL_SIZE_GL = 0.50f;
inline const float EMPTY_VALUE = -10000.0f;
inline const float EPSILON_FLOAT = std::numeric_limits<float>::epsilon();
inline const float TAN_PI_QUARTER = std::tan(45.0f);
inline const float CONE_VISION = 60.0f;
inline const float HALF_CONE_VISION = CONE_VISION / 2.0f;
inline const float DOUBLE_CONE_VISION = CONE_VISION * 2.0f;

inline const uint32_t RAYCAST_LINE_NUMBER = 200, RAYCAST_DEPTH = 20,
RAYCAST_GROUND_CEILING_NUMBER = RAYCAST_LINE_NUMBER / 2.0f;
inline const float SCREEN_VERT_BACKGROUND_GL_STEP = 1.0f / RAYCAST_GROUND_CEILING_NUMBER;
inline const float SCREEN_HORIZ_BACKGROUND_GL_STEP = 2.0f / RAYCAST_LINE_NUMBER;

inline const std::string STR_PLAYER_LIFE = "LIFE::";
inline const std::string STR_PLAYER_AMMO = "AMMO::";
inline const std::string SHADER_DIR_STR = "../Nietsneflow3d/Ressources/Shader/";
inline const std::string LEVEL_RESSOURCES_DIR_STR = "../Nietsneflow3d/Ressources/";
inline const std::string TEXTURES_DIR_STR = "../Nietsneflow3d/Ressources/Textures/";
inline const std::string MENU_ENTRIES = "RETURN TO GAME\\NEW GAME\\EXIT GAME";
inline const std::map<Shader_e, std::string> SHADER_ID_MAP
{
    {Shader_e::COLOR_S, "ceilingFloor"},
    {Shader_e::TEXTURE_S, "wall"}
};

inline std::array<uint32_t, static_cast<uint32_t>(WeaponsType_e::TOTAL)> MAX_WEAPONS_AMMO =
{1, 100, 50};
