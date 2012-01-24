include(../LinuxBuild/AppConfig.pri)

LIBS += $$IPSIUS_LIB_DIR/libDomain.a

#include(../LinuxBuild/LibDomain/LibDomain.pri)
include(../LinuxBuild/LibBoardService/SafeBiProto.pri)
include(../LinuxBuild/LibBoardService/SafeBiProtoExt.pri)
include(../LinuxBuild/iMedia.pri)
include(../LinuxBuild/iDSP.pri)
include(../LinuxBuild/SndMix.pri)
include(../LinuxBuild/AllTests.pri)



SOURCES += main.cpp

