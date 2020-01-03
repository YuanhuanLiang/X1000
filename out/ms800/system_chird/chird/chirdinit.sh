#! /bin/sh

chd=chird

ln -sf /$chd/bin/* /bin/
ln -sf /$chd/sbin/* /sbin/
ln -sf /$chd/lib/* /lib/
#ln -sf /$chd/etc/* /etc/

### sensor configuration ###
#sensor.sh

cfg_sync_bybootenv.sh
#wifiinit.sh

