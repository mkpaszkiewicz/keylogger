ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m := keylogger.o

else
# normal makefile
all: daemon
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

daemon: daemon.c
	gcc daemon.c -o daemon -std=c99

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm daemon

endif
