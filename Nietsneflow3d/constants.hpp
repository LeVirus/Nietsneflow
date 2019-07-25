#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>
#include <map>

enum Components_e
{
    NUMBER_VERTEX_COMPONENT = 0,
    POSITION_VERTEX_COMPONENT,
    SPRITE_TEXTURE_COMPONENT,
    COLOR_VERTEX_COMPONENT,
    TOTAL_COMPONENTS
};

enum Systems_e
{
    COLOR_DISPLAY_SYSTEM
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
    CEILING_FLOOR = 0,
    WALL,
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
    {CEILING_FLOOR, "ceilingFloor"},
    {WALL, "wall"},
    {STATIC_ELEMENT, "staticElement"}
};
#endif // CONSTANTS_HPP

