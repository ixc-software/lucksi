TEMPLATE = lib
CONFIG += qt debug staticlib

DEPENDPATH = .	\
INCLUDEPATH = . \

SOURCES = \
	ioapi.c 	\
	miniunz.c 	\
	mztools.c 	\
	unzip.c		\
	zip.c		\

LIBS += /usr/local/lib/libz.a

TARGET = unzip


