#! /bin/sh

staen="`chd_cfg_get sta.cfg sta_enable`"

ifconfig wlan0 down
sleep 1
killall wpa_supplicant
killall hostapd
killall udhcpd
killall udhcpc

if [ "$staen" = "1" -o "$1" = "sta" ];then
	wpa_supplicant -iwlan0 -Dnl80211 -c/etc/wpa_supplicant.conf -B
	sleep 1

	udhcpc -iwlan0 &
else
	ifconfig wlan0 up
	sleep 1
	
	if [ ! -e /var/lib/misc ];then
		mkdir -p /var/lib/misc
	fi
	if [ ! -e /var/lib/misc/udhcpd.leases ];then
		touch /var/lib/misc/udhcpd.leases
		chmod +x /var/lib/misc/udhcpd.leases
	fi

	hostapd /etc/hostapd.conf &
	ifconfig wlan0 192.168.100.254 netmask 255.255.255.0

	udhcpd -fS /etc/udhcpd.conf &
fi
