#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>
#include <map>

enum Components_e
{
    POSITION_COMPONENT = 0,
    TARGET_PICTURE_DATA_COMPONENT
};

enum DisplayType_e
{
    COLOR,
    TEXTURE
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

const std::string SHADER_DIR_STR = "../Nietsneflow3d/Ressources/Shader/";
const std::string LEVEL_RESSOURCES_DIR_STR = "../Nietsneflow3d/Ressources/";
const std::string TEXTURES_DIR_STR = "../Nietsneflow3d/Ressources/Textures/";
const std::map<Shader_e, std::string> SHADER_ID_MAP
{
    {CEILING_FLOOR, "ceilingFloor"},
    {WALL, "wall"},
    {STATIC_ELEMENT, "staticElement"},
};
#endif // CONSTANTS_HPP

