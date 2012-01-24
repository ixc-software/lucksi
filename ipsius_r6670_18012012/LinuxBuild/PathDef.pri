
linux-g++ {
    IPSIUS_ENV = /media/i686-gcc-env/IpsiusEnv

    INCLUDEPATH += $$IPSIUS_ENV/include \
        $$IPSIUS_ENV/include/qserialport

    ZLIB_PATH = $$IPSIUS_ENV/lib
    QSERIALLIB_PATH = $$IPSIUS_ENV/lib
    RESIPLIB_PATH = $$IPSIUS_ENV/lib
    BOOSTLIB_PATH = $$IPSIUS_ENV/lib
    IBERTY_PATH=/usr/lib/
    ARES_LIB = $$RESIPLIB_PATH/libcares.a
}

linux-g++-64 {
#    CRYPTOPPDIR=/usr/lib

    IPSIUS_ENV = /home/pnom/Works/IpsiusEnv/output/install
    QMAKE_LIBDIR += $$IPSIUS_ENV/lib

    INCLUDEPATH += $$IPSIUS_ENV/include \
        $$IPSIUS_ENV/include/qserialport

    IBERTY_PATH=/usr/lib/
    ARES_LIB = $$RESIPLIB_PATH/libares.a
}


linux-arm-gnueabi-g++ {
    IPSIUS_ENV = /opt/IpsiusEnv/arm-none-linux-gnueabi

    INCLUDEPATH += $$IPSIUS_ENV/include \
        $$IPSIUS_ENV/include/qserialport

    ZLIB_PATH = $$IPSIUS_ENV/lib
    QSERIALLIB_PATH = $$IPSIUS_ENV/lib
    RESIPLIB_PATH = $$IPSIUS_ENV/lib
    BOOSTLIB_PATH = $$IPSIUS_ENV/lib
    ARES_LIB = $$RESIPLIB_PATH/libcares.a
    IBERTY_PATH = /usr/local/arm/4.2.2-eabi/usr/arm-unknown-linux-gnueabi/lib
}

linux-arm-g++ {
#    IPSIUS_ENV = /opt/IpsiusEnv/arm-unknown-linux-uclibcgnueabi
    IPSIUS_ENV = /media/buildroot/urbetter-buildroot-2011.11/IpsiusEnv
    ALSADIR =  /media/buildroot/urbetter-buildroot-2011.11/buildroot-2011.08/output/host/usr/arm-unknown-linux-uclibcgnueabi/sysroot/usr

    INCLUDEPATH += $$IPSIUS_ENV/include \
        $$IPSIUS_ENV/include/qserialport \
        $$ALSADIR/include

    ZLIB_PATH = $$IPSIUS_ENV/lib
    QSERIALLIB_PATH = $$IPSIUS_ENV/lib
    RESIPLIB_PATH = $$IPSIUS_ENV/lib
    BOOSTLIB_PATH = $$IPSIUS_ENV/lib
#    IBERTY_PATH = /usr/local/arm/4.2.2-eabi/usr/arm-unknown-linux-gnueabi/lib
    ARES_LIB = $$RESIPLIB_PATH/libcares.a
}


win32 {
    error("Not supported")
}
