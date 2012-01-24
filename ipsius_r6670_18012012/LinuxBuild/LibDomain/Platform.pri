
SOURCES += Platform/PlatformUtils.cpp \

win32 {

    SOURCES += \
        Platform/PlatformWin32.cpp \
        Platform/PlatformWin32.cpp \
        Platform/SimpleStackTracker.cpp \
        Platform/StackWalker.cpp \
        Platform/platformthread.cpp \
}

unix {
    SOURCES +=  \
        Platform/PlatformPOSIX.cpp \
        Platform/POSIXProcessInfo.cpp \
        Platform/platformthread.cpp

    # to display got from stack function name correctly (in gcc)
    QMAKE_LFLAGS += -rdynamic
}

linux-g++ : SOURCES += Platform/PlatformGlib.cpp

linux-arm-gnueabi-g++ : SOURCES += Platform/PlatformGlib.cpp

linux-arm-g++ : SOURCES += Platform/PlatformUClib.cpp


