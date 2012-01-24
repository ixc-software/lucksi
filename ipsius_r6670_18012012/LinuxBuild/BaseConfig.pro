QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare

QTMOCADVANCED=1
QTMOCINCLUDES="stdafx.h"

# -------------------------------------------------------------------------------------
!exists(PathDef.pri) {
    message("PathDef.pri not found. You can create '../LinuxBuild/PathDef.pri' from '../LinuxBuild/_PathDef.pri'")
}

include(PathDef.pri)

# -------------------------------------------------------------------------------------
# define platform

linux-g++:TARGET_DIR_PREFIX = linux-gcc
linux-arm-gnueabi-g++ :TARGET_DIR_PREFIX = linux-kit-arm
linux-arm-g++ :TARGET_DIR_PREFIX = linux-buildroot

win32:TARGET_DIR_PREFIX = win32

# -------------------------------------------------------------------------------------
#  type make

# ensure one "debug" or "release" in CONFIG so they can be used as
#   conditionals instead of writing "CONFIG(debug, debug|release)"...

CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
    TYPE_MAKE = debug
}

CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
    TYPE_MAKE = release
}

OBJECTS_DIR	= ./bin/$$TARGET_DIR_PREFIX/$$TYPE_MAKE/obj
MOC_DIR     = ./bin/$$TARGET_DIR_PREFIX/$$TYPE_MAKE/generatedfiles

# all libs will be in this directory
IPSIUS_LIB_DIR = ../Lib/$$TARGET_DIR_PREFIX/$$TYPE_MAKE

CONFIG(staticlib) {
    DESTDIR = $$IPSIUS_LIB_DIR
} else {
    DESTDIR =  ./bin/$$TARGET_DIR_PREFIX/$$TYPE_MAKE
}

MAKEFILE = Make$$TARGET_DIR_PREFIX$$TYPE_MAKE
#MAKEFILE = make$$TARGET$$TARGET_DIR_PREFIX$$TYPE_MAKE

# -------------------------------------------------------------------------------------
# application defines

QT += core
QT -= gui

DEFINES	+= HAVE_DECL_BASENAME

CONFIG += console

CONFIG += warn_on

CONFIG(precompile_header):  PRECOMPILED_HEADER = stdafx.h

DEPENDPATH = ../Src

INCLUDEPATH += . \
    ./cfg		 \
    ../Src		 \
    ../Blackfin



