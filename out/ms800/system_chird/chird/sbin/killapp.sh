#! /bin/sh

echo "'killapp.sh [mv]' will move /chird/bin/chd_wmpbox to /chird/ "

if [ "$1" = "mv" ]; then
	mv /chird/bin/chd_wmpbox /chird/
fi

kill -9 `cat /var/run/chd_wmpbox.pid`

