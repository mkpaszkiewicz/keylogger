ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m := keylogger.o

else
# normal makefile
all: daemon
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

daemon: daemon.c server_communication.c protocol_messages.c
	gcc -pthread -std=c99 $^ -o $@

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm daemon

endif
