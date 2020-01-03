#! /bin/sh

ssid="$1"
key="$2"

if [ "$3" = "" ];then
	channel="`chd_cfg_get ap.cfg ap_channel`"
else
	channel="$3"
fi


file="/etc/hostapd.conf"

echo "ssid=$ssid" > $file
echo "wpa_passphrase=$key" >> $file
echo "channel=$channel" >> $file
echo "wpa=2" >> $file
echo "wpa_key_mgmt=WPA-PSK" >> $file
echo "wpa_pairwise=CCMP" >> $file
echo "interface=wlan0" >> $file
echo "driver=nl80211" >> $file
echo "hw_mode=g" >> $file
echo "max_num_sta=20" >> $file
echo "beacon_int=100" >> $file
echo "dtim_period=2" >> $file

#echo "ctrl_interface=/usr/bin/hostapd" >> $file
#echo "ctrl_interface_group=0" >> $file

exit 0
