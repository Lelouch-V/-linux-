ifneq ($(KERNELRELEASE),)

obj-m :=cardevice.o cardriver.o

else
KDIR :=/home/hi3518/hi3518_sdk/Hi3518E_SDK_V1.0.3.0/osdrv/opensource/kernel/linux-3.4.y

ARCH_TYPE = arm
COMPILE = /opt/hisi-linux/x86-arm/arm-hisiv300-linux/target/bin/arm-hisiv300-linux-

all:
        make CROSS_COMPILE=$(COMPILE) ARCH=$(ARCH_TYPE) -C $(KDIR)  M=$(PWD)  modules
        gcc client.c -o client
        arm-hisiv300-linux-gcc server.c -o server
clean:
       make CROSS_COMPILE=$(COMPILE) ARCH=$(ARCH_TYPE) -C $(KDIR)  M=$(PWD)  clean 

endif

