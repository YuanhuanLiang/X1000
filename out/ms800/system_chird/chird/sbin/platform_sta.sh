#! /bin/sh
ssid="$1"
key="$2"
auth="$3"
encrypt="$4"

file="/etc/wpa_supplicant.conf"

echo "network={" > $file
echo "    ssid=\"$ssid\"" >> $file
if [ "$key" = "" ];then
        echo "    key_mgmt=NONE">> $file
else
        echo "    psk=\"$key\"" >> $file
fi
        echo "    scan_ssid=1" >> $file
echo "}" >> $file
echo "network={" >> $file
echo "    ssid=\"mhj_hk_bak\"" >> $file
echo "    psk=\"mhj124578\"" >> $file
echo "}" >> $file

exit 0
