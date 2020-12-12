obj-m := pcd_main.o
ARCH = arm
KERN_LOCAL_DIR = /lib/modules/$(shell uname -r)/build
#KERN_LOCAL_DIR = /lib/modules/linux-4.19.161/build


all:
	make -C $(KERN_LOCAL_DIR) M=$(PWD) modules
clean:
	make -C $(KERN_LOCAL_DIR) M=$(PWD) clean
arm:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules
arm_clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean

