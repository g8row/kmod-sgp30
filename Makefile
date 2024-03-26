obj-m += i2c-sgp30.o
PWD = $(shell pwd)
KDIR = /home/alex/rpi-kernel-6.8/linux

all: module dt
	echo Built Device Tree Overlay and kernel module

module:
	make M=$(PWD) -C $(KDIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules
dt: sgp30.dts
	dtc -@ -I dts -O dtb -o sgp30.dtbo sgp30.dts
clean:
	make -C $(KDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- clean
	rm -rf sgp30.dtbo
	
