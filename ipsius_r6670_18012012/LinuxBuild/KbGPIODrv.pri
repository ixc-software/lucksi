include(../LinuxBuild/ModArmIo.pri)

HEADERS += \
    iPult/KbGPIO/NObjKbGPIOTest.h

SOURCES += \
    iPult/KbGPIO/KbGPIODriver.cpp

!contains(IPSIUS_MODULE, Pult) {
    SOURCES +=  iPult/KbEmul/ButtonTimingPrc.cpp
}
