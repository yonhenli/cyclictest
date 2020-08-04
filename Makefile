obj-m += hrtimer.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all: modules

modules:
	make -C /usr/src/linux-headers-$(shell uname -r) M=$(PWD) modules

insert:
	insmod hrtimer.ko delay_us=4000

remove:
	rmmod hrtimer

.PHONY: clean
clean:
	make -C /usr/src/linux-headers-$(shell uname -r) M=$(PWD) clean
