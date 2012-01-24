TEMPLATE	= app
TARGET		= moc

DEFINES	       += QT_MOC
DESTDIR         = ./bin
INCLUDEPATH	+= . 
DEPENDPATH	+= .
LIBS	        =
OBJECTS_DIR	= ./obj


DEPENDPATH += ./Src \
    ../Src

INCLUDEPATH = \
    ./Src       \
    ../Src      \
    ./cfg       \
    $(QTDIR)    \
    $(QTDIR)/include            \
    $(QTDIR)/include/QtCore     \
    $(BOOSTDIR)

include(moc.pri)
include(mocExtention.pri)

QT_BUILD_TREE=$(QTDIR)
QT_SOURCE_TREE=$(QTDIR)

HEADERS += $(QTDIR)/src/corelib/tools/qdatetime_p.h
#SOURCES += main.cpp

include($(QTDIR)/src/tools/bootstrap/bootstrap.pri)
LIBS += $(QTDIR)/src/tools/bootstrap/libbootstrap.a




target.path=./
INSTALLS += target
include($(QTDIR)/src/qt_targets.pri)
