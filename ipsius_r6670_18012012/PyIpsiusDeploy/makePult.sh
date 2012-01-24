#!/bin/sh

#QTDIR=/usr/Qt_4.7.3_arm 
INSTALL_PATH=$IPSIUS_RELEASE/IpsiusPacks/linux_arm

echo "Run..."
# relese package for pult. No redistr (moved to sdcard_fs)
python3 ./src/main.py  -clean -pack  $INSTALL_PATH -tags bin doc arm

