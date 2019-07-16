TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++17 -lstdc++fs
QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Wpedantic -Og
INCLUDEPATH += includesLib
LIBS += -L ../Nietsneflow3d/lib -lECS -lglad -ldl -lglfw3 -lX11 -lXxf86vm -lXrandr -pthread -lXi

SOURCES += main.cpp \
    Game.cpp \
    MainEngine.cpp \
    LevelManager.cpp \
    GraphicEngine.cpp \
    Level.cpp \
    PictureData.cpp \
    OpenGLUtils/Shader.cpp \
    OpenGLUtils/Texture.cpp

HEADERS += \
    Game.hpp \
    MainEngine.hpp \
    LevelManager.hpp \
    GraphicEngine.h \
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
    ECS/Components/positioncomp.hpp \
    ECS/Components/targetpicturedata.hpp \
    ECS/Components/spritetexturedata.hpp \
    OpenGLUtils/Shader.hpp \
    includesLib/glad.h \
    OpenGLUtils/Texture.hpp

