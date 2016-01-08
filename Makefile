ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := keylogger.o

else
# normal makefile
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

endif
