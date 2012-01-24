
INCLUDEPATH += $$CRYPTOPPDIR

linux-g++:LIBS += $$CRYPTOPPDIR/libcryptopp.a

!linux-g++: error("'Crypto.pri'. Not supported")

HEADERS += Utils/CryptUtils.h

include(../LinuxBuild/Pcid.pri)


