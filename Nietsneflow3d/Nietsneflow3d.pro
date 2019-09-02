TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Wpedantic -Og
INCLUDEPATH += includesLib
LIBS += -L../Nietsneflow3d/lib  -lECS -lglad -ldl -lglfw3 -lX11 -lXxf86vm -lXrandr -pthread -lXi

SOURCES += main.cpp \
    Game.cpp \
    MainEngine.cpp \
    LevelManager.cpp \
    GraphicEngine.cpp \
    Level.cpp \
    PictureData.cpp \
    OpenGLUtils/Shader.cpp \
    OpenGLUtils/Texture.cpp \
    ECS/ECSManager.cpp \
    ECS/Systems/ColorDisplaySystem.cpp \
    OpenGLUtils/VerticesData.cpp \
    ECS/Systems/MapDisplaySystem.cpp \
    PhysicalEngine.cpp \
    ECS/Systems/InputSystem.cpp \
    ECS/Systems/CollisionSystem.cpp \
    CollisionUtils.cpp \
    ECS/Systems/FirstPersonDisplaySystem.cpp

HEADERS += \
    Game.hpp \
    MainEngine.hpp \
    LevelManager.hpp \
    ECS/includesLib/component.hpp \
    ECS/includesLib/componentmanager.hpp \
    ECS/includesLib/ECSconstantes.hpp \
    ECS/includesLib/engine.hpp \
    ECS/includesLib/entity.hpp \
    ECS/includesLib/system.hpp \
    ECS/includesLib/systemmanager.hpp \
    includesLib/inireader.h \
    includesLib/BaseECS/component.hpp \
    includesLib/BaseECS/componentmanager.hpp \
    includesLib/BaseECS/engine.hpp \
    includesLib/BaseECS/entity.hpp \
    includesLib/BaseECS/system.hpp \
    includesLib/BaseECS/systemmanager.hpp \
    Level.hpp \
    PictureData.hpp \
    constants.hpp \
    OpenGLUtils/Shader.hpp \
    includesLib/glad.h \
    OpenGLUtils/Texture.hpp \
    includesLib/glfw3.h \
    ECS/Components/PositionVertexComponent.hpp \
    ECS/Components/SpriteTextureComponent.hpp \
    ECS/Components/ColorVertexComponent.hpp \
    ECS/ECSManager.hpp \
    GraphicEngine.hpp \
    ECS/Systems/ColorDisplaySystem.hpp \
    OpenGLUtils/glheaders.hpp \
    OpenGLUtils/VerticesData.hpp \
    ECS/Systems/MapDisplaySystem.hpp \
    ECS/Components/StaticElementComponent.hpp \
    ECS/Components/MoveableComponent.hpp \
    ECS/Components/MapCoordComponent.hpp \
    PhysicalEngine.hpp \
    ECS/Systems/InputSystem.hpp \
    ECS/Components/InputComponent.hpp \
    ECS/Systems/CollisionSystem.hpp \
    ECS/Components/CircleCollisionComponent.hpp \
    ECS/Components/RectangleCollisionComponent.hpp \
    ECS/Components/LineCollisionComponent.hpp \
    CollisionUtils.hpp \
    ECS/Components/GeneralCollisionComponent.hpp \
    ECS/Systems/FirstPersonDisplaySystem.hpp

