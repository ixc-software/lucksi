#!/bin/bash

export PATH=/home/pnom/Works/Arm/buildroot/urbetter_buildroot-2011.08/output/host/usr/bin/:$PATH

#export KERNEL_DIR = /usr/src/linux-headers-2.6.38-8
export KERNEL_DIR=/home/pnom/Works/Arm/buildroot/urbetter_buildroot-2011.08/output/build/linux-custom

#CC = gcc
export CC=arm-unknown-linux-uclibcgnueabi-gcc

make