TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS_RELEASE += -Ofast
QMAKE_CFLAGS_RELEASE += -march=native
QMAKE_CXXFLAGS_RELEASE += -march=native
QMAKE_CXXFLAGS_RELEASE += -static-libgcc -static-libstdc++

DESTDIR = $$PWD/bin
#DEFINES += INIDEBUG

include(catch/catch.pri)
include(../pge_gamesave.pri)

LIBS += -lsqlite3

SOURCES += \
    gamesave_test.cpp

