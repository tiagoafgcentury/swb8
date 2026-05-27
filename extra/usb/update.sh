#!/bin/sh

mount -o remount,rw /sda1/sda1 /media/sda1 && cd /media/sda1 && rm mbgui-0.0.0.0 && wget http://192.168.30.7/montage/mbgui-0.0.0.0 && sync && mount -o remount,ro /media/sda1 && reboot
