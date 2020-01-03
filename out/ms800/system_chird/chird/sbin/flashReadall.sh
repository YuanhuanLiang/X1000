#! /bin/sh

mkdir /var/mkimg

dd if=/dev/mtdblock0 of=/var/mkimg/uboot
cat /var/mkimg/uboot > /upgrade_image.flash
rm /var/mkimg/uboot

dd if=/dev/mtdblock1 of=/var/mkimg/kernel
cat /var/mkimg/kernel >> /upgrade_image.flash
rm /var/mkimg/kernel

dd if=/dev/mtdblock2 of=/var/mkimg/system
cat /var/mkimg/system >> /upgrade_image.flash
rm /var/mkimg/system

