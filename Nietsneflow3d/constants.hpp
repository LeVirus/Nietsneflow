#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

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

const std::string SHADER_DIR_STR = "../Nietsneflow3d/Ressources/Shader/";
const std::string LEVEL_RESSOURCES_DIR_STR = "../Nietsneflow3d/Ressources/Level1/";
#endif // CONSTANTS_HPP

