ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := keylogger.o
keylogger-y := keylogger.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

endif
