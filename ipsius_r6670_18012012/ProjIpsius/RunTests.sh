#!/bin/sh

echo "Run auto tests..."
/YOUR_PATH_TO/ipsius_r6670_18012012/ProjIpsius-build-desktop/bin/linux-gcc/release/ProjIpsius -n Ipsius -i /YOUR_PATH_TO/ipsius_r6670_18012012/ProjIpsius/isc/AutoTest/ -la 56001 -af Main.isc -at

