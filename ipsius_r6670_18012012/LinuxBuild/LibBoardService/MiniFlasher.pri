#include(../LinuxBuild/LibUart/iUart.inc)

HEADERS += MiniFlasher/NObjMiniFlasher.h

SOURCES += \
    MiniFlasher/NObjMiniFlasher.cpp	\
	MiniFlasher/MfUtils.cpp			\
	MiniFlasher/MfProcess.cpp		\
    MiniFlasher/LoadImgImpl.cpp		\
    MiniFlasher/UartBootImpl.cpp	\
        
SOURCES += \
        ../Blackfin/Src/MiniFlasher/MfClient.cpp 	\
        ../Blackfin/Src/MiniFlasher/MfTrace.cpp		\
        ../Blackfin/Src/MiniFlasher/MfPacketFactory.cpp \

