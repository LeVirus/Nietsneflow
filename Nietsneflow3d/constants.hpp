#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>
#include <map>

enum Components_e
{
    POSITION_VERTEX_COMPONENT = 0,
    SPRITE_TEXTURE_COMPONENT,
    COLOR_VERTEX_COMPONENT,
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
    GROUND,
    CEILING,
    OBJECT
};

enum Shader_e
{
    COLOR_S = 0,
    TEXTURE_S,
    STATIC_ELEMENT,
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


const std::string SHADER_DIR_STR = "../Nietsneflow3d/Ressources/Shader/";
const std::string LEVEL_RESSOURCES_DIR_STR = "../Nietsneflow3d/Ressources/";
const std::string TEXTURES_DIR_STR = "../Nietsneflow3d/Ressources/Textures/";
const std::map<Shader_e, std::string> SHADER_ID_MAP
{
    {Shader_e::COLOR_S, "ceilingFloor"},
    {Shader_e::TEXTURE_S, "wall"},
    {Shader_e::STATIC_ELEMENT, "staticElement"}
};

const std::map<Texture_t, std::string> TEXTURE_ID_PATH_MAP
{
    {Texture_t::WALL_T, "Global.png"},
    {Texture_t::GLOBAL, "walltest.jpg"}
};
#endif // CONSTANTS_HPP

