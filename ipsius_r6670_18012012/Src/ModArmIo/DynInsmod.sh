#module="ModArmIo"
#device="ModArmIo"

module=$(awk -F'"' '/CName/ {print $2}' SharedConsts.h)
device=$(awk -F'"' '/CName/ {print $2}' SharedConsts.h)

insmod ./$module.ko

rm -f /dev/$device

major=`awk "\\$2==\"$module\" {print \\$1}" /proc/devices`

mknod /dev/$device c $major 0

