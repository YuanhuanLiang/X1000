#! /bin/sh

while true
do
	synctime="`date | grep 1970`"
	if [ -n "$synctime" ]; then
        	sleep 5
	else
		sleep 120
	fi

	### ntpd sync time ###
	ntpd -n -q
done

