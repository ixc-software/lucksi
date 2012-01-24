#KERNEL_DIR = /usr/src/linux-headers-2.6.38-8
KERNEL_DIR = /home/valkiriy/_ArmFiles/urbetter-linux2.6.28-v1.0

#CC = gcc
CC = arm-none-linux-gnueabi-gcc

obj-m += mydrv.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
