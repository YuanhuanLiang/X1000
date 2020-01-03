#! /bin/sh

kill -USR1 `cat /var/run/chd_wmpbox.pid`

sysled=`chd_cfg_get gpio.cfg gpio_sys`
chd_gpio -n $sysled -d out -l 1
