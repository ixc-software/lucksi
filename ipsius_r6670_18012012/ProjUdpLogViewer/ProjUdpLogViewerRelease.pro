include(ProjUdpLogViewer.pri)

CONFIG += qt release

DESTDIR = Bin/Release

unix{

    OBJECTS_DIR = LinuxRelease/
    MOC_DIR = LinuxRelease/Generated/
    RCC_DIR = LinuxRelease/Generated/

}

win32{

    OBJECTS_DIR = Release/
    MOC_DIR = Release/Generated/
    RCC_DIR = Release/Generated/
} 
