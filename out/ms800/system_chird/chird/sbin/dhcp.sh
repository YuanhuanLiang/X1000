#! /system/bin/sh

Usage()
{
	echo "Usage: dhcp.sh enable < 1 | 0 >"
	echo "       dhcp.sh get <option>"
	echo "       dhcp.sh set [gateway] [start] [end]"
	echo "dhcp enable"
	echo "       1 : open dhcp server"
	echo "       0 : close dhcp server"
	echo "   Example:"
	echo "       # dhcp.sh enable 1"
	echo ""
	echo "get dhcp information:"
	echo "   option:"
	echo "       enable  	-- dhcp enable"
	echo "       gateway	-- wifi gateway"
	echo "       start   	-- dhcp distribution start ip"
	echo "       end    	-- dhcp distribution end ip"
	echo "   Example:"
	echo "       # dhcp.sh get gateway"
	echo ""
	echo "set dhcp server:"
	echo "   Example:"
	echo "       1. If set '[start]',but don't set '[gateway]' you can set like: "
	echo "       # dhcp.sh set NULL 100"
	echo "       2. If set '[end]',but don't set '[gateway] [start]' you can set like:"
	echo "       # dhcp.sh set NULL NULL 200"
	exit 1
}

if [ "$1" != "enable" -a "$1" != "get" -a "$1" != "set" ]; then
	Usage
fi

if [ "$1" = "enable" -a $# -eq 2 ]; then
	enable="$2"
	if [ "$enable" = "1" ]; then
		chd_cfg_set dhcp.cfg dhcp_enable 1
	elif [ "$enable" = "0" ]; then
		chd_cfg_set dhcp.cfg dhcp_enable 0
	else
		Usage
	fi
#	platform_dhcp.sh $enable
elif [ "$1" = "get" -a $# -eq 2 ]; then
	option="$2"
	case $option in 
	enable)
		echo "`chd_cfg_get dhcp.cfg dhcp_enable`";;
	gateway)
		echo "`chd_cfg_get dhcp.cfg dhcp_gateway`";;
	start)
		echo "`chd_cfg_get dhcp.cfg dhcp_start`";;
	end)
		echo "`chd_cfg_get dhcp.cfg dhcp_end`";;
	*)
		echo "Error: $option not found"
		exit 1 ;;
	esac
elif [ "$1" = "set" -a $# -ge 2 ]; then
	gateway="$2"
	start="$3"
	end="$4"

#	platform_dhcp.sh "set" $gateway $start $end

	if [ -n "$gateway" -a "$gateway" != "NULL" -a "`chd_cfg_get dhcp.cfg dhcp_gateway`" != "$gateway" ]; then
		chd_cfg_set dhcp.cfg dhcp_gateway "$gateway"
	fi

	if [ -n "$start" -a "$start" != "NULL" -a "`chd_cfg_get dhcp.cfg dhcp_start`" != "$start" ]; then
		chd_cfg_set dhcp.cfg dhcp_start "$start"
	fi

	if [ -n "$end" -a "$end" != "NULL" -a "`chd_cfg_get dhcp.cfg dhcp_end`" != "$end" ]; then
		chd_cfg_set dhcp.cfg dhcp_end "$end"
	fi
else
	Usage
fi

exit 0

