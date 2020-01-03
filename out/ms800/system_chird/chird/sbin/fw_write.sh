#! /bin/sh

chd=chird

if [ "$1" == "" ]; then
	echo "fw_write.sh < Firmware Name >"
	echo "Firmware name list:"
	echo "    uboot"
	echo "    boot"
	echo "    system"
	exit 1
else
	sysled=`chd_cfg_get gpio.cfg gpio_sys`
	chd_gpio -n $sysled -d out -l 300 &

	if [ "$1" == "uboot" ]	
		dd if=$1.bin of=/dev/mtdblock0
	elif [ "$1" == "boot" ]	
		dd if=$1.bin of=/dev/mtdblock1
	elif [ "$1" == "system" ]	
		dd if=$1.bin of=/dev/mtdblock2
	fi
	exit 0
fi
