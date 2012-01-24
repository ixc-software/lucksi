FORMS = GUI/udplogviewergui.ui

HEADERS += \ 
    GUI/udplogviewergui.h \
    GUI/UdpLogViewer.h \
    GUI/UdpLogViewerGuiHelpers.h \
    GUI/UlvGuiLogWidget.h \
    GUI/UlvGuiLogSrcWidget.h \
    GUI/UlvGuiLogToQImage.h \
    GUI/UlvGuiStartConfig.h \
    GUI/UlvGuiImageRender.h \
    GUI/QtDeclaredMetaTypes.h \
    GUI/UlvGuiCtrlToModelParams.h \
    GUI/UlvGuiLogViewRender.h \
    GUI/UlvGuiScrollWidget.h \
    GUI/IUdpLogViewerGui.h

SOURCES += \
    GUI/udplogviewergui.cpp \
    GUI/UdpLogViewer.cpp \
    GUI/UdpLogViewerGuiHelpers.cpp \
    GUI/UlvGuiLogWidget.cpp \
    GUI/UlvGuiLogSrcWidget.cpp \
    GUI/UlvGuiLogToQImage.cpp \
    GUI/UlvGuiStartConfig.cpp \
    GUI/UlvGuiImageRender.cpp \
    GUI/UlvGuiCtrlToModelParams.cpp \

#LIBS += $(QTDIR)/lib/libQtGui.a
