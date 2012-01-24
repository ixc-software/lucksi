include(ProjUdpLogViewer.pri)

CONFIG += qt debug

DESTDIR = Bin/Debug

unix{

    OBJECTS_DIR = LinuxDebug/
    MOC_DIR = LinuxDebug/Generated/
    RCC_DIR = LinuxDebug/Generated/

}

win32{

    OBJECTS_DIR = Debug/
    MOC_DIR = Debug/Generated/
    RCC_DIR = Debug/Generated/
} 
