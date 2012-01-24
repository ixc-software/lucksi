#!/bin/sh

export LD_LIBRARY_PATH="./bin/"
export QT_QWS_FONTDIR="./bin/fonts"
export QT_PLUGIN_PATH="./bin/"

./bin/ProjIpsius -n Ipsius -i ../isc -ta 50200 -la 56001 -af Aoz.isc -at ! -qws -nomouse