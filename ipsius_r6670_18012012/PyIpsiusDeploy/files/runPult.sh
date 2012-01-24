#!/bin/sh

#export LD_LIBRARY_PATH="/mnt/sdcard/bin/"
#export QT_QWS_FONTDIR="/mnt/sdcard/bin/fonts"
export QT_PLUGIN_PATH="mnt/sdcard/bin/"

./bin/ProjIpsius -n Ipsius -i ../isc -ta 50200 -la 56001 -af pult.isc -at ! -qws -nomouse