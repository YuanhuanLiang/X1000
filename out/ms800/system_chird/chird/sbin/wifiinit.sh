#! /bin/sh


ap_en=`chd_cfg_get ap.cfg ap_enable`
if [ "$ap_en" = "1" ]; then
	chd_cfg_set sta.cfg sta_enable 0
	ssid=`chd_cfg_get ap.cfg ap_ssid`
	key=`chd_cfg_get ap.cfg ap_key`
	
	platform_ap.sh "$ssid" "$key"
	killwifi.sh ap
else
	chd_cfg_set sta.cfg sta_enable 1 
	ssid=`chd_cfg_get sta.cfg sta_ssid`
	key=`chd_cfg_get sta.cfg sta_key`
	auth=`chd_cfg_get sta.cfg sta_authmode`
	encrypt=`chd_cfg_get sta.cfg sta_encrypt`

	platform_sta.sh	"$ssid" "$key" "$auth" "$encrypt"
	killwifi.sh sta
fi


exit 0
