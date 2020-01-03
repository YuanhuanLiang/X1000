#! /bin/sh

Usage()
{
	echo "Usage : sta.sh get <option>"
	echo "        sta.sh set <ssid> [key] [<IP> [netmask] [gateway] [dns]]"
	echo "get STA information:"
	echo "   option:"
	echo "        enable     -- STA enable "
	echo "        ssid       -- STA ssid"
	echo "        key        -- STA key"
	echo "        mode       -- STA ip allocation mode, DHCP or STATIC"
	echo "        authmode   -- STA authmode"
	echo "        encrypt    -- STA encrypt"
	echo "        ip         -- STA static ip"
	echo "        netmask    -- STA static netmask"
	echo "        gateway    -- STA static gateway"
	echo "        dns        -- STA static dns"
	echo "        macaddr    -- STA macaddr"
	echo "        status     -- STA connection status"
	echo "   Example:"
	echo "        # sta.sh get ssid"
	echo ""
	echo "set STA information: "
	echo "        <key>      : If need, the string length must >= 8"
	echo "   Example:" 
	echo "         1. If use the 'DHCP' mode, you don't need to set,you can set like: "
	echo "         # sta.sh set myssid 12345678 "
	echo ""
	echo "         2. If use the 'STATIC' mode, you must set the '<IP>':"
	echo "         # sta.sh set myssid 12345678 192.168.1.1"
	echo ""
	echo "         3. **note** : '[netmask] [gateway] [dns]' is optional, if you want to set"
	echo "         '[dns]',but do not want to set '[netmask] [gateway]', you can set like: "
	echo "         # sta.sh set myssid 12345678 192.168.1.1 NULL NULL 8.8.8.8"
	echo ""
	exit 1
}


if [ "$1" != "get" -a  "$1" != "set" ]; then
	Usage
fi

if [ "$1" = "get" -a $# -eq 2 ]; then
	option="$2"

	case $option in
	enable)
		echo "`chd_cfg_get sta.cfg sta_enable`";;
	ssid)
		echo "`chd_cfg_get sta.cfg sta_ssid`";;
	key)
		echo "`chd_cfg_get sta.cfg sta_key`";;
	mode)
		echo "`chd_cfg_get sta.cfg sta_mode`";;
	authmode)
		echo "`chd_cfg_get sta.cfg sta_authmode`";;
	encrypt)
		echo "`chd_cfg_get sta.cfg sta_encrypt`";;
	ip)
		echo "`chd_cfg_get sta.cfg sta_ip`";;
	netmask)
		echo "`chd_cfg_get sta.cfg sta_netmask`";;
	gateway)
		echo "`chd_cfg_get sta.cfg sta_gateway`";;
	dns)
		echo "`chd_cfg_get sta.cfg sta_dns`";;
	macaddr)
		echo "`chd_cfg_get sta.cfg sta_macaddr`";;
	status)
		platform_wifi_status_get.sh;;
	*)
		echo "Error: $option not found.."
		exit 1
		;;
	esac

elif [ "$1" = "set" -a $# -ge 2 ]; then
	ssid="$2"
	key="$3"
	ip="$4"
	nm="$5"
	gw="$6"
	dns="$7"

	############# check DHCP/STATIC mode ################
	if [ -n "$ip" ]; then
		mode="STATIC"
	else
		mode="DHCP"
	fi
	
	######### If the directory does not exist, make it ########
	if [ ! -d "/var/run/" ]; then
		mkdir -p /var/run/
	fi
	
	######### If the file dose not exist, make it #########
	if [ "$key" = "" -o "$auth" = "NONE" ]; then
		key=""
	fi
	
	### In order to prevent 'AutoWLAN.sh' reload xradio_wlan driver, set a global variables ###
	platform_sta.sh "$ssid" "$key" "$auth" "$encrypt" "$mode" "$ip" "$nm" "$gw" "$dns"
	killwifi.sh sta
	if [ "`chd_cfg_get system.cfg system_StaQRscan`" = "1" ]; then
		chd_cfg_set system.cfg system_StaQRscan 0
	fi

	if [ "`chd_cfg_get sta.cfg sta_monitor`" = "1" ]; then
		chd_cfg_set sta.cfg sta_monitor 0
	fi

	if [ "`chd_cfg_get sta.cfg sta_enable`" != "1" ]; then
		chd_cfg_set sta.cfg sta_enable 1
	fi

	if [ "`chd_cfg_get ap.cfg ap_enable`" != "0" ]; then
		chd_cfg_set ap.cfg ap_enable 0
	fi

	if [ "$ssid" != "`chd_cfg_get sta.cfg sta_ssid`" ]; then
		chd_cfg_set sta.cfg sta_ssid "$ssid"
	fi

	if [ "$key" != "`chd_cfg_get sta.cfg sta_key`" ]; then
		chd_cfg_set sta.cfg sta_key "$key"
	fi

	if [ "$auth" != "`chd_cfg_get sta.cfg sta_authmode`" ]; then
		chd_cfg_set sta.cfg sta_authmode "$auth"
	fi

	if [ "$encrypt" != "`chd_cfg_get sta.cfg sta_encrypt`" ]; then
		chd_cfg_set sta.cfg sta_encrypt  "$encrypt"
	fi
	
	if [ "$mode" != "`chd_cfg_get sta.cfg sta_mode`" ]; then
		chd_cfg_set sta.cfg sta_mode "$mode"
	fi
		
	if [ -n "$ip" -a "`chd_cfg_get sta.cfg sta_ip`" != "$ip" ]; then
		chd_cfg_set sta.cfg sta_ip "$ip"
	fi
	
	if [ -n "$nm" ] && [ "$nm" != "NULL" -a "`chd_cfg_get sta.cfg sta_netmask`" != "$nm" ]; then
		chd_cfg_set sta.cfg sta_netmask "$nm"
	fi
	
	if [ -n "$gw" ] && [ "$gw" != "NULL" -a "`chd_cfg_get sta.cfg sta_gateway`" != "$gw" ]; then
		chd_cfg_set sta.cfg sta_gateway "$gw"
	fi
	
	if [ -n "$dns" ] && [ "$dns" != "NULL" -a "`chd_cfg_get sta.cfg sta_dns`" != "$dns" ]; then
		chd_cfg_set sta.cfg sta_dns "$dns"
	fi

	if [ -n "$mac" -a "`chd_cfg_get sta.cfg sta_macaddr`" != "$mac" ]; then
		chd_cfg_set sta.cfg sta_macaddr "$mac"
	fi
else
	Usage
fi

exit 0


