#! /bin/sh

chd=chird
#wifiled=`chd_cfg_get gpio.cfg gpio_wifi`
sysled=`chd_cfg_get gpio.cfg gpio_sys`
randen=`chd_cfg_get system.cfg system_randPWD`

chd_cfg_set /$chd/config/other.cfg envchange 1

if [ "$randen" = "1" ];then
	mkrand
fi
## clear wpa_supplicant.conf, and remove the p2p_supplicant.conf ##
rm -rf /config/*
#sensor.sh
cfg_sync_bybootenv.sh
wifiinit.sh

### send the signel to chd_wmpbox, chd_wmpbox will set GPIO to describe the device status###
if [ -f "/var/run/chd_wmpbox.pid" ]; then
        ### send SIGHUP signal to chd_wmpbox ###
        kill -SIGHUP `cat /var/run/chd_wmpbox.pid`
        ### or use " kill -HUP `cat /var/run/chd_wmpbox.pid` "
fi
#chd_gpio -n $wifiled -d out -l 30 &
chd_gpio -n $sysled -d out -l 30 &

mhjreset.sh


kill -9 `cat /var/run/chd_devface.pid`
kill -9 `cat /var/run/chd_wmpbox.pid`
#rm /var/run/wifiscan.inf

exit 0
