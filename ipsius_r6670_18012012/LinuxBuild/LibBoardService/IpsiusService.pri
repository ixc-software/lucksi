INCLUDEPATH += ./src/

HEADERS += \
    IpsiusService/AppLauncherImpl.h     \
	IpsiusService/HwBoardFsmStates.h	\
	IpsiusService/NObjBoardFilter.h 	\
	IpsiusService/NObjBooterAutoUpdater.h	\
	IpsiusService/NObjHwBoard.h 		\
	IpsiusService/NObjHwBoardList.h 	\
	IpsiusService/NObjHwFinder.h		\

SOURCES += \
	IpsiusService/HwFirmware.cpp 		\
	IpsiusService/HwFirmwareStore.cpp	\
	IpsiusService/NObjBoardFilter.cpp	\
	IpsiusService/NObjBooterAutoUpdater.cpp	\
	IpsiusService/NObjHwBoard.cpp		\
	IpsiusService/NObjHwFinder.cpp		\


contains(IPSIUS_MODULE, Gui) {
    SOURCES += IpsiusService/AppLauncherGui.cpp
} else {
    SOURCES += IpsiusService/AppLauncher.cpp
}
