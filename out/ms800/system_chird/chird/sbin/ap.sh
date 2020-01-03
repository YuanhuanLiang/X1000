#! /bin/sh

Usage()
{
	echo "Usage: ap.sh get <option>"
	echo "       ap.sh set <ssid> <key> [channel] [hidden] [txpower] [htmode]"
	echo "get AP information:"
	echo "   option:"
	echo "        enable    -- wifi AP enable "
	echo "        ssid      -- AP ssid"
	echo "        key       -- AP key"
	echo "        authmode  -- AP authmode"
	echo "        encrypt   -- AP encrypt"
	echo "        channel   -- AP channel"
	echo "        txpower   -- AP txpower"
	echo "        hidden    -- AP hidden"
	echo "        htmode    -- AP htmode"
	echo "        macaddr   -- AP device macaddr"
	echo "        status    -- AP status get"
	echo "   Example:"
	echo "        # ap.sh get ssid"
	echo ""
	echo "set AP information: "
	echo "        <key>     : The string length must >= 8 "
	echo "        [channel] : 1 ~ 13"
	echo "        [hidden]  : '1' for hidden AP ssid,  '0' for broadcast AP ssid"
	echo "        [txpower] : 0 ~ 100"
	echo "        [htmode]  : '1' for 20/40MHz, '0' for 20MHz"
	echo "   Example:"
	echo "        # ap.sh set myssid 12345678 10 0 80 1 "
	exit 1
}

if [ "$1" != "get" -a "$1" != "set" ]; then
	Usage
fi

	
if [ "$1" = "get" -a $# -eq 2 ]; then
	option="$2"
	case $option in
	macaddr)
		platform_wifimac_get.sh ;;
	enable)
		echo "`chd_cfg_get ap.cfg ap_enable`";;
	ssid)
		echo "`chd_cfg_get ap.cfg ap_ssid`";;
	key)
		echo "`chd_cfg_get ap.cfg ap_key`";;
	authmode)
		echo "`chd_cfg_get ap.cfg ap_authmode`";;
	encrypt)
		echo "`chd_cfg_get ap.cfg ap_encrypt`";;
	channel)
		echo "`chd_cfg_get ap.cfg ap_channel`";;
	txpower)
		echo "`chd_cfg_get ap.cfg ap_txpower`";;
	hidden)
		echo "`chd_cfg_get ap.cfg ap_hidden`";;
	htmode)
		echo "`chd_cfg_get ap.cfg ap_htmode`";;
	status)
		platform_wifi_status_get.sh;;
	*)
		echo "Error: $option not found.."
		exit 1;;
	esac
elif [ "$1" = "set" -a $# -ge 3 ]; then
	ssid="$2"
	key="$3"
	channel="$4"
	hidden="$5"
	txpower="$6"
	htmode="$7"
	
	# check <key> length
	keylen=`expr length "$key"`
	if [ $keylen -lt 8 ]; then
		Usage
	fi

	if [ "`chd_cfg_get sta.cfg sta_monitor`" = "1" ]; then
		chd_cfg_set sta.cfg sta_monitor 0
	fi

	if [ "`chd_cfg_get sta.cfg sta_enable`" = "1" ]; then
		chd_cfg_set sta.cfg sta_enable 0
	fi

	if [ "`chd_cfg_get ap.cfg ap_enable`" != "1" ]; then
		chd_cfg_set ap.cfg ap_enable 1
	fi

	if [ "`chd_cfg_get ap.cfg ap_ssid`" != "$ssid" ]; then
		chd_cfg_set ap.cfg ap_ssid "$ssid"
	fi

	if [ "`chd_cfg_get ap.cfg ap_key`" != "$key" ]; then
		chd_cfg_set ap.cfg ap_key "$key"
	fi

	if [ -n "$channel" ] && [ $channel -gt 0 -a $channel -lt 14 ]; then
		if [ "`chd_cfg_get ap.cfg ap_channel`" != "$channel" ]; then
			chd_cfg_set ap.cfg ap_channel "$channel"
		fi
	fi

	if [ -n "$hidden" ] && [ "$hidden" = "1" -o "$hidden" = "0" ]; then
		if [ "`chd_cfg_get ap.cfg ap_hidden`" != "$hidden" ]; then
			chd_cfg_set ap.cfg ap_hidden "$hidden"
		fi
	fi

	
	if [ -n "$txpower" ] && [ $txpower -gt 0 -a $txpower -le 100 ]; then
		if [ "`chd_cfg_get ap.cfg ap_txpower`" != "$txpower" ]; then
			chd_cfg_set ap.cfg ap_txpower "$txpower"
		fi
	fi
	
	if [ -n "$htmode" ] && [ "$htmode" = "1" -o "$htmode" = "0" ]; then
		if [ "`chd_cfg_get ap.cfg ap_htmode`" != "$htmode" ]; then
			chd_cfg_set ap.cfg ap_htmode "$htmode"
		fi
	fi

	platform_ap.sh "$ssid" "$key" "$channel" "$hidden" "$txpower" "$htmode"

	killwifi.sh

else
	Usage
fi

exit 0

