#! /bin/sh
######################################
# if '2' wireless driver is not insmod 
# if '1' ap up/sta connected;
# if '0' ap down/sta not connected
######################################


wlanko_insmod="`wifi_reload_driver.sh lsmod`"
if [ ! -n "$wlanko_insmod" ];then
	echo "2"
	exit 0
fi

wireless=`chd_cfg_get other.cfg wireless`

if [ "$wireless"  = "RTL8189ES" -o "$wireless"  = "RTL8188EU" ];then
	if [ ! -f "/sys/class/net/wlan0/operstate" ];then
	        echo "0"
	        exit 1 
	fi
	value0=`cat /sys/class/net/wlan0/operstate`
	value1=`cat /sys/class/net/wlan1/operstate`
	if [ "$value0" = "up" -o "$value1" = "up" ];then
	        echo "1"
	else
	        echo "0"
	fi
else

	sta_en="`chd_cfg_get sta.cfg sta_enable`"
	route="`/chird/bin/chd_route wlan0`"
		
	if [ "$sta_en" = "1" ]; then
        	info="`iw dev wlan0 link | grep Connected`"
    		if [ -n "$info" -a "$route" = "1" ]; then
            		echo "1"
    		else
            		echo "0"
    		fi
	else
    		if [ "$route" = "1" ]; then
            		echo "1"
    		else
            		echo "0"
    		fi
	fi
fi
