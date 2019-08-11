#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>
#include <map>

using pairUI_t = std::pair<uint32_t, uint32_t>;
using pairFloat_t = std::pair<float, float>;

enum Components_e
{
    POSITION_VERTEX_COMPONENT = 0,
    SPRITE_TEXTURE_COMPONENT,
    COLOR_VERTEX_COMPONENT,
    MAP_COORD_COMPONENT,
    STATIC_ELEMENT_COMPONENT,
    MOVEABLE_COMPONENT,
    TOTAL_COMPONENTS
};

enum Systems_e
{
    COLOR_DISPLAY_SYSTEM,
    MAP_DISPLAY_SYSTEM
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

enum BaseShapeType_e
{
    TRIANGLE,
    RECTANGLE,
    NONE
};

const float LEVEL_TILE_SIZE_PX = 30.0f;
const float MAP_LOCAL_CENTER_X_GL = 0.75f;
const float MAP_LOCAL_CENTER_Y_GL = -0.75f;
const float MAP_LOCAL_SIZE_GL = 0.50f;

const std::string SHADER_DIR_STR = "../Nietsneflow3d/Ressources/Shader/";
const std::string LEVEL_RESSOURCES_DIR_STR = "../Nietsneflow3d/Ressources/";
const std::string TEXTURES_DIR_STR = "../Nietsneflow3d/Ressources/Textures/";
const std::map<Shader_e, std::string> SHADER_ID_MAP
{
    {Shader_e::COLOR_S, "ceilingFloor"},
    {Shader_e::TEXTURE_S, "wall"}
};

const std::map<Texture_t, std::string> TEXTURE_ID_PATH_MAP
{
    {Texture_t::GLOBAL, "Global.png"},
    {Texture_t::WALL_T, "walltest.jpg"}
};
#endif // CONSTANTS_HPP

