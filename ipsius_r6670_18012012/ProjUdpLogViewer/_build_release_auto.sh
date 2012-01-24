#!/bin/sh

echo "Building ProjUdpLogViewer ..."

echo "Update build info"
python3 ../Blackfin/PyBfTools/src/BuildInfoHeader.py ../ProjUdpLogViewer/BuildInfo.h ../ProjUdpLogViewer/.svn

echo "Generate makefile"
qmake ProjUdpLogViewer.pro -r CONFIG+=release -spec linux-g++ -o MakefileRelease

echo "Building ..."

make -f MakefileRelease 
