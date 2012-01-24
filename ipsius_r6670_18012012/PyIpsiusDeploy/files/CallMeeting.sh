#!/bin/sh

export LD_LIBRARY_PATH="./bin"

./bin/ProjIpsius -n Ipsius -i ../isc -ta 50200 -la 56001 -af CallMeeting.isc -at
