#!/bin/sh

export IPSIUS_PC_ENV=/home/pnom/Works/IpsiusEnv
export QTDIR=$IPSIUS_PC_ENV/output/build/qt-everywhere-opensource-src-4.7.4
export BOOSTDIR=$IPSIUS_PC_ENV/output/install/include

$QTDIR/bin/qmake ./moc.pro

make

