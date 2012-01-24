!contains(IPSIUS_MODULE, Gui) : error("iPult.pri: 'Gui' not defined. Wrong 'AppLauncher.cpp' will be included to make")

# add qt libs
QT += multimedia
QT += gui

# -------------------------------------------------------------------------------------
# KbEmul

HEADERS += \
    iPult/KbEmul/ButtonTimingPrc.h       \
    iPult/KbEmul/KbEmulWindow.h          \
    iPult/KbEmul/KbPosWidget.h           \
    iPult/KbEmul/KbLedWidget.h           \
    iPult/KbEmul/KbProfile.h             \
    iPult/KbEmul/NObjPultKbEmulProbe.h



SOURCES += \
    iPult/KbEmul/ButtonTimingPrc.cpp     \
    iPult/KbEmul/KbLedWidget.cpp         \
    iPult/KbEmul/KbProfile.cpp           \
    iPult/KbEmul/NObjPultKbEmulProbe.cpp \
    iPult/KbEmul/KbEmulWindow.cpp        \
    iPult/KbEmul/KbPosWidget.cpp         \
    iPult/KbEmul/KeyBoardEmul.cpp        \
    #iPult/KbGPIO/KbGPIODriver.cpp

# -------------------------------------------------------------------------------------
# KbMcu

HEADERS += iPult/KbMcu/NObjPultKbMcuProbe.h

SOURCES += iPult/KbMcu/NObjPultKbMcuProbe.cpp

# -------------------------------------------------------------------------------------
# PcView

HEADERS += \
    iPult/PcView/GuiViewBootState.h     \
    iPult/PcView/GuiViewInitialState.h  \
    iPult/PcView/GuiViewCallState.h     \
    iPult/PcView/GuiViewMessageState.h  \
    iPult/PcView/GuiViewDialState.h     \
    iPult/PcView/GuiViewVolumeState.h   \
    iPult/PcView/mainviewwidget.h       \
    iPult/PcView/NObjViewDebug.h        \
    iPult/PcView/StateStatusWidget.h    \
    iPult/PcView/ViewPcDebugerWidget.h

SOURCES += \
    iPult/PcView/GuiViewCallState.cpp   \
    iPult/PcView/NObjViewDebug.cpp      \
    iPult/PcView/StateStatusWidget.cpp  \
    iPult/PcView/Utils.cpp              \
    iPult/PcView/ViewPcDebugerWidget.cpp \
    iPult/PcView/ViewTheme.cpp \
    iPult/PcView/PopUpMenuWd.cpp \
    iPult/PcView/SkButtonWd.cpp

# -------------------------------------------------------------------------------------
# SndIo

    HEADERS += \
        iPult/SndIO/QtAudioInput.h   \
        iPult/SndIO/QtAudioOutput.h  \
#        iPult/SndIO/SndIoNull.h

    SOURCES += \
        iPult/SndIO/SndIoNull.cpp \
        iPult/SndIO/SndIoQt.cpp

    contains(IPSIUS_MODULE, AlsaSound) {
        SOURCES += iPult/SndIO/SndIoAlsa.cpp \
            iPult/SndIO/SndIoAlsaLib.cpp \
            iPult/SndIO/SndIoQtExt.cpp
    } else {
        SOURCES += iPult/SndIO/SndIoAlsa_null.cpp \
            iPult/SndIO/SndIoQtExt_null.cpp
    }

# -------------------------------------------------------------------------------------
# SndEmul

HEADERS += iPult/SndEmul/NObjEmulSoundCreator.h

SOURCES += iPult/SndEmul/CmpEmulSoundDrv.cpp

# -------------------------------------------------------------------------------------
# SndCmp

HEADERS += \
    iPult/SndCmp/NObjCmpSoundCreator.h  \
    iPult/SndCmp/NObjPultBoardSettings.h

SOURCES += \
    iPult/SndCmp/CmpPointMn.cpp \
    iPult/SndCmp/CmpSoundDrv.cpp

# -------------------------------------------------------------------------------------
# SndLocal

HEADERS += iPult/SndLocal/NObjLocalSoundDrv.h \
    iPult/SndLocal/NObjQtSoundSettings.h

SOURCES += \    
    iPult/SndLocal/LocalConvPoint.cpp       \
    iPult/SndLocal/LocalRtpPoint.cpp        \
    iPult/SndLocal/LocalSoundDrv.cpp        \
    iPult/SndLocal/LocalSoundUtils.cpp      \
    iPult/SndLocal/NObjLocalSoundDrv.cpp    \

# -------------------------------------------------------------------------------------
# SndNull

HEADERS += iPult/SndNull/NObjNullSoundCreator.h

SOURCES += iPult/SndNull/NullSoundDrv.cpp

# -------------------------------------------------------------------------------------
# SndShare

HEADERS += iPult/SndShare/PultSoundDrvCreator.h

SOURCES += \
    iPult/SndShare/PultSoundDrvCreator.cpp  \
    iPult/SndShare/SndPointList.cpp

# -------------------------------------------------------------------------------------
# States
SOURCES += \
    iPult/States/StateCall.cpp      \
    iPult/States/SwitchState.cpp    \
    iPult/States/StateSndTest.cpp   \
    iPult/States/UiSkBar.cpp

# -------------------------------------------------------------------------------------
# Call

HEADERS +=  \
    iPult/Call/LpMode.h \
    iPult/Call/PultCall.h\
    iPult/CallMng.h

SOURCES += \
    iPult/Call/PultCall.cpp         \
    iPult/Call/LocalPoint.cpp       \
    iPult/Call/ConversationPoint.cpp\
    iPult/Call/CallMng.cpp


# -------------------------------------------------------------------------------------
# Test

contains(IPSIUS_MODULE, AlsaSound) {
    HEADERS +=  iPult/Test/NObjSoundTest.h

    SOURCES += iPult/Test/NObjSoundTest.cpp
}
# -------------------------------------------------------------------------------------
# Pult dir

HEADERS += \    
    iPult/CofidecState.h              \
    iPult/NObjKbEmulSettings.h        \
    iPult/NObjKbMcuSettings.h         \
    iPult/NObjKbTimingProf.h          \
    iPult/NObjNumDialerProfile.h      \
    iPult/NObjPult.h                  \
    iPult/NObjPultBody.h              \
    iPult/NObjPultDirKeyBinds.h       \
    iPult/NObjPultPcViewSettings.h    \
    iPult/NObjPultPhBook.h            \
    iPult/NObjPultSettings.h          \
    iPult/PultKeyConst.h              \


SOURCES += \    
    iPult/ConvApi.cpp             \    
    iPult/DirKeysBind.cpp         \
    iPult/KbEventFilter.cpp       \
    iPult/KeyboardLayout.cpp      \
    iPult/NObjPultBody.cpp        \    
    iPult/PultTest.cpp            \
    iPult/KbLayoutProf.cpp        \    
    iPult/NObjPult.cpp            \
    iPult/pultkeyconst.cpp        \
    iPult/PultViewExTest.cpp      \


