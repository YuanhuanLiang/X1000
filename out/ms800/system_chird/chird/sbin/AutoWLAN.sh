#! /system/bin/sh

### auto open/closs wlan0 ###	

while true
do
	wlanko_insmod="`wifi_reload_driver.sh lsmod`"

	if [ -f "/sys/class/net/usb0/operstate" -o -f "/sys/class/net/eth0/operstate" -o -e /dev/ttyUSB0 ]; then
		if [ -n "$wlanko_insmod" ];then
			ifconfig wlan0 down
			wifi_reload_driver.sh rmmod
		fi
	else
		### if 'sta.sh' is setting, don't do this command ###
		if [ ! -n "$wlanko_insmod" ];then
			wifi_reload_driver.sh insmod
			kill -9 `cat /var/run/chd_wifi.pid`
		fi
	fi

	sleep 5
done
