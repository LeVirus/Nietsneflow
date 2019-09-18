#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>
#include <map>

using pairUI_t = std::pair<uint32_t, uint32_t>;
using pairLong_t = std::pair<long, long>;
using pairFloat_t = std::pair<float, float>;
using pairDouble_t = std::pair<double, double>;

enum Components_e
{
    POSITION_VERTEX_COMPONENT = 0,
    SPRITE_TEXTURE_COMPONENT,
    COLOR_VERTEX_COMPONENT,
    MAP_COORD_COMPONENT,
    INPUT_COMPONENT,
    CIRCLE_COLLISION_COMPONENT,
    LINE_COLLISION_COMPONENT,
    RECTANGLE_COLLISION_COMPONENT,
    STATIC_ELEMENT_COMPONENT,
    GENERAL_COLLISION_COMPONENT,
    MOVEABLE_COMPONENT,
    TOTAL_COMPONENTS
};

enum Systems_e
{
    COLOR_DISPLAY_SYSTEM,
    MAP_DISPLAY_SYSTEM,
    INPUT_SYSTEM,
    COLLISION_SYSTEM,
    FIRST_PERSON_DISPLAY_SYSTEM
};

enum DisplayType_e
{
    COLOR,
    TEXTURE,
    BOTH
};

enum Direction_e
{
    NORTH,
    WEST,
    SOUTH,
    EAST
};

enum MoveOrientation_e
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

enum EnemySpriteType_e
{
    STATIC,
    ATTACK,
    MOVE,
    DYING
};

enum LevelStaticElementType_e
{
    GROUND = 0,
    CEILING,
    OBJECT
};

enum Shader_e
{
    COLOR_S = 0,
    TEXTURE_S,
    TOTAL_SHADER
};

enum Texture_t
{
    WALL_T,
    GLOBAL, // TMP
    TOTAL_TEXTURE
};

enum BaseShapeTypeGL_e
{
    TRIANGLE,
    RECTANGLE,
    NONE
};

enum CollisionTag_e
{
    PLAYER,
    WALL_C,
    ENEMY,
    BULLET_PLAYER,
    BULLET_ENEMY,
    OBJECT_C
};

enum CollisionShape_e
{
    CIRCLE,
    RECTANGLE_C,
    SEGMENT
};

inline const float PI = 3.14159265359f;
inline const float PLAYER_RAY_DISPLAY = 0.03f;
inline const float LEVEL_TILE_SIZE_PX = 30.0f;
inline const float LEVEL_HALF_TILE_SIZE_PX = LEVEL_TILE_SIZE_PX / 2;
inline const float ENEMY_RAY = LEVEL_HALF_TILE_SIZE_PX;
inline const float MAP_LOCAL_CENTER_X_GL = 0.75f;
inline const float MAP_LOCAL_CENTER_Y_GL = -0.75f;
inline const float MAP_LOCAL_SIZE_GL = 0.50f;

inline const std::string SHADER_DIR_STR = "../Nietsneflow3d/Ressources/Shader/";
inline const std::string LEVEL_RESSOURCES_DIR_STR = "../Nietsneflow3d/Ressources/";
inline const std::string TEXTURES_DIR_STR = "../Nietsneflow3d/Ressources/Textures/";
inline const std::map<Shader_e, std::string> SHADER_ID_MAP
{
    {Shader_e::COLOR_S, "ceilingFloor"},
    {Shader_e::TEXTURE_S, "wall"}
};

inline const std::map<Texture_t, std::string> TEXTURE_ID_PATH_MAP
{
    {Texture_t::GLOBAL, "Global.png"},
    {Texture_t::WALL_T, "walltest.jpg"}
};
#endif // CONSTANTS_HPP

