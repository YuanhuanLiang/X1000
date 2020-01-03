#!/bin/sh

value=`ping $1 -w 4 | grep time`

if [ "$value" == "" ]; then
 	### "ping again"
 	value=`ping $1 -w 8 | grep time`
	if [ "$value" == "" ]; then
		echo "0"
		###  "restart chd_wifi chd_devface will restart chd_wifi"  
		#killwifi.sh
	else
		echo "1"
	fi
else
	echo "1" 
fi
