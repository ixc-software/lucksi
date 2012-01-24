#!/bin/sh

export LD_LIBRARY_PATH="./bin"

# Setup network settings on board
# $1 -> board address
# $2 -> UseDHCP
# $3 -> IP
# $4 -> Gateway
# $5 -> NetMask
# $6 -> Password

# $@ - using all parameters from 1 to N

./bin/ProjIpsius -n Ipsius -la 56001 -v -i ../isc -sc SetNetwork.isc -x $@