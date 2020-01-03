#!/bin/sh
#
# Start camera....
#
case "$1" in
start)
echo "Starting camera..."
if [ -d /sys/class/android_usb/android0 ]
then
cd /sys/class/android_usb/android0
value=`cat ./functions`
if [ "$value" != "webcam" ]
then
echo 0 > enable
echo 18d1 > idVendor
echo d002 > idProduct
echo webcam > functions
echo 1 > enable
fi
cd -
else
echo "notice : mass storage and adb don't use, kernel config error"
fi
;;
stop)
;;
restart|reload)
;;
*)
echo "Usage: $0 {start|stop|restart}"
exit 1
esac
exit $?

