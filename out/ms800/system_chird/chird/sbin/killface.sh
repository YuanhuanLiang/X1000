#! /system/bin/sh

echo "'killface.sh [mv]' will move /chird/bin/chd_devface to /chird/ "

if [ "$1" = "mv" ]; then
	mv /chird/bin/chd_devface /chird/
fi

kill -9 `cat /var/run/chd_devface.pid`

