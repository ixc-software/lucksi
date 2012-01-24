IPSIUS_MODULE += Gui

include(../LinuxBuild/AppConfig.pri)

DEFINES	+= HAVE_DECL_BASENAME


TARGET	= UdpLogViewer

CONFIG += precompile_header warn_on
PRECOMPILED_HEADER = stdafx.h

DEPENDPATH = ../Src
INCLUDEPATH = .		\
    ./cfg		\
    ../Src		\
    /usr/include

RC_FILE = ./UdpLogViewerGUI.rc
RESOURCES = ./UdpLogViewerGUI.qrc

include(GUI/GUI.pri)
include(Core/Core.pri)
include(Tests/Tests.pri)

include(../LinuxBuild/LibDomain/LibDomain.pri)
include(../LinuxBuild/LibDomain/Utils.pri)
include(../LinuxBuild/LibDomain/Platform.pri)


SOURCES += main.cpp
