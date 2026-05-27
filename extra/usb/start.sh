#!/bin/sh

BASEDIR=`dirname $0`

mount -o remount,ro /media/sda1

LD_LIBRARY_PATH=$BASEDIR:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

#ifconfig eth0 192.168.70.10 netmask 255.255.255.0
/sbin/udhcpc eth0

time $BASEDIR/mbgui-0.0.0.0 -l

echo "Update SAT-Monitor"

time wget 'https://eng.web.centurybr.com.br/Process' -O /dev/null
#time wget 'http://10.0.11.4:3011/Process' -O /dev/null

echo -n "Done update SAT-Monitor: "
date '+%Y-%m-%d %H:%M'

cd /tmp
umount -l /media/sda1

sleep 10m
reboot
