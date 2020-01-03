#! /bin/sh

chd=chird

envchange=`chd_cfg_get /$chd/config/other.cfg envchange`
mipiRegf=`chd_cfg_get /$chd/config/other.cfg sensor_mipi_regfile`
cvbsRegf=`chd_cfg_get /$chd/config/other.cfg sensor_cvbs_regfile`

mipifilename="mipi_sensor_"$mipiRegf".cfg"
cvbsfilename="cvbs_sensor_"$cvbsRegf".cfg"

if [ "$envchange" = "1" ]; then
        if [ -f /$chd/config/sensor/$mipifilename ];then
                cp -f  /$chd/config/sensor/$mipifilename /config/mipi_sensor.cfg
	else
		echo "Not have file $mipifilename"
        fi

        if [ -f /$chd/config/sensor/$cvbsfilename ];then
                cp -f  /$chd/config/sensor/$cvbsfilename /config/cvbs_sensor.cfg
	else
		echo "Not have file $cvbsfilename"
        fi
fi
