linux-arm-gnueabi-g++:error("Crypto module not supported on this platform")

HEADERS += \
	Pcid/NObjMachineId.h \

SOURCES += \
	Pcid/DmiDecodeWrap.cpp \
	Pcid/NObjMachineId.cpp \
	Pcid/ProtMachineIdPosix.cpp \
	Pcid/ProtSerial.cpp \
