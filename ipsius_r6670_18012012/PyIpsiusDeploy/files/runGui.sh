#!/bin/sh

export LD_LIBRARY_PATH="./bin"
export PYTHONPATH
PYTHONPATH=./PyLib; #Python/PyQt4

"/usr/bin/python"  "./PyIpsiusQConfig/src/Main/main.py" /e "../../../bin/ProjIpsius" /c "./PyIpsiusQConfig/UserCfg"


