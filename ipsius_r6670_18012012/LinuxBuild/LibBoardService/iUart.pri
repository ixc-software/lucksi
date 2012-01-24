QSERIALDEVICE=$$MISC_PATH/QextSerialPort

unix{
    DEFINES += _TTY_POSIX_
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# -------------------------------------------------
# Source

INCLUDEPATH +=  $$QSERIALDEVICE/posix

SOURCES += \
    iUart/Uart.cpp              \
    iUart/UartProfile.cpp       \
    iUart/UartReturnCode_i.cpp  \

	
linux-g++ {
    SOURCES += iUart/PosixNameUartPort.cpp
}

linux-arm-gnueabi-g++ {
    SOURCES += iUart/ArmNameUartPort.cpp
}

# -------------------------------------------------
# lib

debug:LIBS += $$CONTRIB_LIB/libqextserialportd.a
release:LIBS += $$CONTRIB_LIB/libqextserialport.a
