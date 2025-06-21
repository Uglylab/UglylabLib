QT += core

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    rule.cpp \
    simulator.cpp \
    world.cpp

HEADERS += \
    grid.h \
    grid3d.h \
    ispecies.h \
    rule.h \
    simulator.h \
    species.h \
    uglylab_sharedmemory.h \
    vec3.h \
    world.h

LIBS += -pthread


RESOURCES += \
    resources.qrc

DISTFILES += \
    LICENSE \
    README.md
