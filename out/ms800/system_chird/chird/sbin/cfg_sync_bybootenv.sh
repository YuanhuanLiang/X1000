#! /bin/sh

chd=chird

envchange=`chd_cfg_get /$chd/config/other.cfg envchange`
ssidpre=`chd_cfg_get /$chd/config/ap.cfg ap_ssid`

if [ "$envchange" = "1" ]; then
	cp -rf /$chd/config/* /config/
	ssid=$ssidpre'_'$(cat /sys/class/net/wlan0/address|awk -F ":" '{print $4":"$5":"$6 }'| tr a-z A-Z)

	chd_cfg_set ap.cfg ap_ssid "$ssid"
	chd_cfg_set /$chd/config/other.cfg envchange 0
fi

exit 0
