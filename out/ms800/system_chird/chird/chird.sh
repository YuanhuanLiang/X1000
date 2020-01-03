#! /bin/sh

#cpuset.sh

staEn=`chd_cfg_get sta.cfg sta_enable`
timesyncEn=`chd_cfg_get system.cfg system_timesyncEn`

if [ "$staEn" = "1" ];then
	sta_mode=`chd_cfg_get sta.cfg sta_mode`
	if [ "$sta_mode" = "STATIC" ]; then
		ip=`chd_cfg_get sta.cfg sta_ip`
		ifconfig wlan0 $ip
	fi
fi

### if do, maybe have error when run 'sta.sh set xxxx xxxxx'
#/chird/sbin/wifiscan.sh

if [ "$timesyncEn" = "1" ];then
	time_sync.sh &
fi

manualreboot=`chd_cfg_get system.cfg system_ManualrebootFlag`
if [ "$manualreboot" = "1" ];then
	chd_cfg_set system.cfg system_ManualrebootFlag 0
fi

