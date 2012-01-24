# include(iUart.inc)

unix{
    DEFINES += _TTY_POSIX_
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# -------------------------------------------------
# Source

SOURCES += \
    iUart/Uart.cpp              \
    iUart/UartProfile.cpp       \
    iUart/UartReturnCode_i.cpp  \

	
linux-g++ : SOURCES += iUart/PosixNameUartPort.cpp

linux-arm-gnueabi-g++ : SOURCES += iUart/ArmNameUartPort.cpp

linux-arm-g++ : SOURCES += iUart/ArmNameUartPort.cpp

# -------------------------------------------------
# lib

LIBS += $$QSERIALLIB_PATH/libqextserialport.a
