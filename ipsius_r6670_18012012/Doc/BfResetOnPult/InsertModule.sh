insmod ./mydrv.ko
mknod /dev/myDev c 251 0 # 251 is major nr, 0 is minor nr.
