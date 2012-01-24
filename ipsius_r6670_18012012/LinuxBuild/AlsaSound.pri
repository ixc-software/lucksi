
HEADERS += \
    AlsaSound/Amixer.h          \
    AlsaSound/Tests.h           \
    AlsaSound/PcmStreamImpl.h   \
    AlsaSound/PcmStreams.h      \
    AlsaSound/PcmStreamParam.h

SOURCES += \
    AlsaSound/PcmStreams.cpp \
    AlsaSound/Amixer.cpp

#---------------------------------
# fore direct use alsa-userlib

LIBS += $$ALSADIR/lib/libasound.so

#BHEADERS += AlsaSound/AlsaWrapper.h

#SOURCES += AlsaWrapper.cpp






