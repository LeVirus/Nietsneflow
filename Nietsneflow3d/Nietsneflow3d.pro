TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Wpedantic -Og
INCLUDEPATH += ./ECS/includesLib
LIBS += -L./ECS/lib -lECS

SOURCES += main.cpp \
    Game.cpp \
    MainEngine.cpp \
    LevelManager.cpp \
    GraphicEngine.cpp

HEADERS += \
    Game.hpp \
    MainEngine.hpp \
    LevelManager.hpp \
    GraphicEngine.h

