#!/bin/sh

outdir=$1
imagedir=$2
keydir=$3
keyname=$4
publickey=$keydir/$keyname.x509.pem
privatekey=$keydir/$keyname.pk8

python -m otapackage --output=$outdir --imgpath=$imagedir --publickey=$publickey --privatekey=$privatekey

