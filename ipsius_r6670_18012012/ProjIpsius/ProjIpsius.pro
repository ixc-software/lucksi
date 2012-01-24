include(../LinuxBuild/AppConfig.pri)

IPSIUS_MODULE +=  Dss1
IPSIUS_MODULE +=  iSip

include(../LinuxBuild/LibBoardEmul/LibBoardEmul.pri)
include(../LinuxBuild/LibBoardService/LibBoardService.pri)
include(../LinuxBuild/LibUart/iUart.inc)
include(../LinuxBuild/LibUart/iUart.pri)
include(../LinuxBuild/LibDomain/LibDomain.pri)
include(../LinuxBuild/Utils.pri)

include(../LinuxBuild/ProjIpsius.pri) # process include modules

include(../LinuxBuild/LibDomain/Platform.pri)

LIBS += -lrt

SOURCES += main.cpp


