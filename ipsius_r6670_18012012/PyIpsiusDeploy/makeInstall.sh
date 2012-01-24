#!/bin/sh

QTDIR=/opt/qtsdk-2010.05.1/qt/
INSTALL_PATH=$IPSIUS_RELEASE/IpsiusPacks/linux
export QTDIR

echo "Run..."
python3 ./src/main.py -b_main -clean -pack  $INSTALL_PATH -tags bin_debug doc fwu redistr_debug
#python3 ./src/main.py -clean -pack  $INSTALL_PATH -tags bin_debug doc fwu redistr_debug
