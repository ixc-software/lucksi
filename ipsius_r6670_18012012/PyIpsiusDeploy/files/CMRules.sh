#!/bin/sh

export LD_LIBRARY_PATH="./bin"

.\bin\ProjIpsius.exe -n Ipsius -i ../isc -af CMRules.isc -at

pause