#!/bin/bash

# usage [release | debug] [clean]

if [ "$1" != "debug" ] && [ "$1" != "release" ]; then
    echo "allowed as argument:"
    echo  "             [debug | release] [clean]"
    exit
fi

export QTDIR=/media/i686-gcc-env/IpsiusEnv
export QTSPEC=linux-g++
export MAKEFILENAME=MakeGcc
export QTMOCADVANCED=1

if [ "$2" = "clean" ]; then
    echo "Cleaning ProjIpsius"
    make clean -f $MAKEFILENAME
fi

#echo "Update build info"
#python3 ../Blackfin/PyBfTools/src/BuildInfoHeader.py ../ProjIpsius/BuildInfo.h ../ProjIpsius/.svn ../Src/iCmp

echo "Run qmake"
if [ "$1" = "debug" ]; then
    $QTDIR/bin/qmake ProjIpsius.pro -r CONFIG+=debug -spec $QTSPEC -o $MAKEFILENAME
else
    $QTDIR/bin/qmake ProjIpsius.pro -r -spec $QTSPEC -o $MAKEFILENAME
fi

echo "Building ..."
make -f $MAKEFILENAME


